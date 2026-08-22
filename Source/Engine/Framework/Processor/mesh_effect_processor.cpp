//===================================================
// File  ：_/Processor/mesh_effect_processor.cpp
// Date  ：2026/08/22
// Author：Miu Kitamura
// 
// ・MeshEffectの更新処理を行うProcessor
//===================================================
#include "mesh_effect_processor.h"

#include "Engine/Core/game_object.h"
#include "Engine/Core/scene_interface.h"
#include "Engine/Device/mi_fps.h"

// Utility
#include "Utility/mi_math.h"
#include "Utility/mi_curve.h"
#include "Utility/mi_string.h"
using namespace MiMath;

#include <algorithm>
#include <cmath>
#include <random>

#include "Engine/Framework/Component/transform_component.h"
#include "Engine/Framework/Component/mesh_effect_component.h"
#include "Engine/engine_service_locator.h"

namespace 
{
    /// @brief TransformModuleを評価して、スケール・回転・ローカル行列を計算する
    void EvaluateTransformModule(
        const MeshEffectData::TransformModule& transformModule,
        float normalizedTime,
        DirectX::XMFLOAT3& outScale,
        DirectX::XMFLOAT3& outRotation,
        DirectX::XMMATRIX& localMatrix)
    {
        outScale = transformModule.scaleOverLifetime.Evaluate(normalizedTime);
        outRotation = transformModule.rotationOverLifetime.Evaluate(normalizedTime);
        XMMATRIX scaleMatrix = XMMatrixScaling(outScale.x, outScale.y, outScale.z);
        XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(
            XMConvertToRadians(outRotation.x),
            XMConvertToRadians(outRotation.y),
            XMConvertToRadians(outRotation.z));
        localMatrix = scaleMatrix * rotationMatrix;
    }

    /// @brief FlipbookModuleを評価して、UVオフセット・UVスケーリングを計算する
    void EvaluateFlipbookModule(
        const MeshEffectData::FlipbookModule& flipbookModule,
        const DirectX::XMFLOAT4& rendererUvRect,
        float currentTime,
        float normalizedTime,
        DirectX::XMFLOAT4& outFrameUvRect)
    {
        const int tileX = (std::max)(flipbookModule.tileX, 1);
        const int tileY = (std::max)(flipbookModule.tileY, 1);
        const int atlasFrameCount = tileX * tileY;
        const int startFrame = std::clamp(flipbookModule.startFrame, 0, atlasFrameCount - 1);
        const int availableFrames = atlasFrameCount - startFrame;
        const int frameCount = std::clamp(flipbookModule.frameCount, 1, availableFrames);
        int animationFrame = 0;

        // TimeModeに応じて現在のフレームを計算
        switch (flipbookModule.timeMode) {
        case MeshEffectData::TimeMode::Lifetime: {
            animationFrame = static_cast<int>(normalizedTime * frameCount);
            break;
        }
        case MeshEffectData::TimeMode::Speed: {
            animationFrame = static_cast<int>(currentTime * (std::max)(flipbookModule.framePerSecond, 0.0f));
            break;
        }
        }

        // 再生モードに応じてフレームを調整
        switch (flipbookModule.playbackMode) {
            case MeshEffectData::FlipbookPlaybackMode::Once: {
                animationFrame = (std::min)(animationFrame, frameCount - 1);
                break;
            }
            case MeshEffectData::FlipbookPlaybackMode::Loop: {
                animationFrame %= frameCount;
                break;
            }
            case MeshEffectData::FlipbookPlaybackMode::PingPong:
            {
                if (frameCount > 1) {
                    const int pingPongFrameCount = frameCount * 2 - 2;
                    animationFrame %= pingPongFrameCount;
                    if (animationFrame >= frameCount) {
                        animationFrame = pingPongFrameCount - animationFrame;
                    }
                }
                else {
                    animationFrame = 0;
                }
                break;
            }
            default: break;
        }

        // UV座標の計算
        const int currentFrame = startFrame + animationFrame;
        const float uSize = 1.0f / tileX;
        const float vSize = 1.0f / tileY;
        int frameX = currentFrame % tileX;
        int frameY = currentFrame / tileX;

        outFrameUvRect.x = rendererUvRect.x + frameX * uSize * rendererUvRect.z;
        outFrameUvRect.y = rendererUvRect.y + frameY * vSize * rendererUvRect.w;
        outFrameUvRect.z = uSize * rendererUvRect.z;
        outFrameUvRect.w = vSize * rendererUvRect.w;
    }
}

void MeshEffectProcessor::Initialize()
{
    m_pendingAssetReloads.clear();
}

void MeshEffectProcessor::Finalize()
{
    m_pendingAssetReloads.clear();
}

void MeshEffectProcessor::Process(IScene* pScene)
{
    if (!pScene) return;
    float deltaTime = FPS_GetDeltaTime();

    auto* meshEffectPool = pScene->GetComponentPool<MeshEffectComponent>();

    // === リロード要求がある場合は処理 ===
    if (meshEffectPool && !m_pendingAssetReloads.empty()) {
        for (const PendingAssetReload& pending : m_pendingAssetReloads) {
            if (!pending.asset) continue;

            for (MeshEffectComponent& meshEffect : meshEffectPool->GetList()) {
                if (meshEffect.GetAsset() != pending.asset) continue;
                meshEffect.GetDesc() = pending.asset->GetDesc();
                meshEffect.SetModelResource(nullptr);
                meshEffect.SetTextureResource(nullptr);
            }
        }
        m_pendingAssetReloads.clear();
    }

    // === MeshEffectの更新処理 ===
    if (!meshEffectPool) return;
    auto& meshEffects = meshEffectPool->GetList();

    for (auto& meshEffect : meshEffects) {
        auto& evaluatedState = meshEffect.EvaluatedState();
        evaluatedState.visible = false;

        if (!meshEffect.GetOwner()->GetActive()) continue;
        if (!meshEffect.GetEnable()) continue;

        // playOnAwakeが有効で、まだ再生されていない場合は再生する
        if (meshEffect.Main().playOnAwake && !meshEffect.HasPlayed()) {
            meshEffect.Play();
        }

        // 再生中でなければスキップ
        if (!meshEffect.IsPlaying()) continue;

        // === シミュレーションの更新 ===
        const float scaledDeltaTime = deltaTime * meshEffect.Main().simulationSpeed;
        float currentTime = meshEffect.GetTime() + scaledDeltaTime;

        // durationを超えたらループするか停止する
        bool inDuration = true;
        if (meshEffect.Main().duration > 0.0f && currentTime >= meshEffect.Main().duration) {
            if (meshEffect.Main().loop) {
                currentTime = std::fmod(currentTime, meshEffect.Main().duration);
            }
            else {
                currentTime = meshEffect.Main().duration;
                inDuration = false;
            }
        }
        meshEffect.SetTime(currentTime);
        if (!inDuration) {
            meshEffect.Stop();
        }

        const float duration = meshEffect.Main().duration;
        const float normalizedTime = duration > 0.0f
            ? std::clamp(currentTime / duration, 0.0f, 1.0f)
            : 0.0f;

        // モジュール無効時に前フレームの値が残らないよう中立値へ戻す
        evaluatedState.scale = { 1.0f, 1.0f, 1.0f };
        evaluatedState.rotation = { 0.0f, 0.0f, 0.0f };
        evaluatedState.localEffectMatrix = XMMatrixIdentity();
        evaluatedState.buffer = {};
        evaluatedState.buffer.frameUVRect = meshEffect.Renderer().uvRect;
        evaluatedState.buffer.effectTime = currentTime;
        evaluatedState.blendMode = meshEffect.Renderer().blendMode;

        // === Transformの更新 ===
        if (meshEffect.Transform().enabled) {
            EvaluateTransformModule(
                meshEffect.Transform(),
                normalizedTime,
                meshEffect.EvaluatedState().scale,
                meshEffect.EvaluatedState().rotation,
                meshEffect.EvaluatedState().localEffectMatrix);
        }

        // === Flipbookの更新 ===
        if (meshEffect.Flipbook().enabled) {
            EvaluateFlipbookModule(
                meshEffect.Flipbook(),
                meshEffect.Renderer().uvRect,
                currentTime,
                normalizedTime,
                evaluatedState.buffer.frameUVRect);
        }

        // === Scrollの更新 ===
        if (meshEffect.Scroll().enabled) {
            evaluatedState.buffer.uvTiling = meshEffect.Scroll().tiling;
            evaluatedState.buffer.uvOffset.x = std::fmod(
                meshEffect.Scroll().offset.x + meshEffect.Scroll().scrollSpeed.x * currentTime,
                1.0f);
            evaluatedState.buffer.uvOffset.y = std::fmod(
                meshEffect.Scroll().offset.y + meshEffect.Scroll().scrollSpeed.y * currentTime,
                1.0f);
        }

        // === Waveの更新 ===
        if (meshEffect.Wave().enabled &&
            meshEffect.Wave().type == MeshEffectData::WaveType::UV) {
            // Waveの評価
            float waveValue = meshEffect.Wave().amplitudeOverLifetime.Evaluate(normalizedTime);
            evaluatedState.buffer.uvWaveDirection = meshEffect.Wave().direction;
            evaluatedState.buffer.uvWaveAmplitude = waveValue;
            evaluatedState.buffer.uvWaveFrequency = meshEffect.Wave().frequency;
            evaluatedState.buffer.uvWaveSpeed = meshEffect.Wave().speed;
        }

        // === Gradientの更新 ===
        if (meshEffect.Gradient().enabled) {
            // Gradientの評価
            evaluatedState.buffer.effectColor = meshEffect.Gradient().color.Evaluate(normalizedTime);
        }

        TextureRepository* textureRepository = Engine::GetTextureRepository();
        ModelRepository* modelRepository = Engine::GetModelRepository();
        if (meshEffect.GetTextureResource() == nullptr && textureRepository) {
            std::string texturePath = meshEffect.Renderer().texturePath;
            if (!texturePath.empty()) {
                meshEffect.SetTextureResource(textureRepository->GetTextureResource(texturePath));
            }
        }
        if (meshEffect.GetModelResource() == nullptr && modelRepository) {
            std::string modelPath = meshEffect.Renderer().modelPath;
            if (!modelPath.empty()) {
                meshEffect.SetModelResource(modelRepository->GetModel(modelPath));
            }
        }

        evaluatedState.visible = meshEffect.IsPlaying() && meshEffect.GetModelResource();
    }
}

/// @brief MeshEffectAssetがリロードされた際に呼び出されるコールバック関数
void MeshEffectProcessor::OnMeshEffectAssetReloaded(
    const std::filesystem::path& path,
    const MeshEffectAsset& asset)
{
    const auto normalizedPath = path.lexically_normal();
    auto it = std::find_if(
        m_pendingAssetReloads.begin(),
        m_pendingAssetReloads.end(),
        [&asset](const PendingAssetReload& pending) {
            return pending.asset == &asset;
        });
    if (it != m_pendingAssetReloads.end()) {
        it->path = normalizedPath;
        return;
    }

    // 新しいリロード要求を追加
    m_pendingAssetReloads.push_back({ normalizedPath, &asset });
}
