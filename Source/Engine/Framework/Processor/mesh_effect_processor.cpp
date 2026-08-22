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
        float normalizedTime,
        DirectX::XMFLOAT2& outUvOffset,
        DirectX::XMFLOAT2& outUvTiling)
    {
        int totalFrames = flipbookModule.tileX * flipbookModule.tileY;
        int currentFrame = 0;

        // TimeModeに応じて現在のフレームを計算
        switch (flipbookModule.timeMode) {
        case MeshEffectData::TimeMode::Lifetime: {
            currentFrame = static_cast<int>(normalizedTime * totalFrames);
            break;
        }
        case MeshEffectData::TimeMode::Speed: {
            currentFrame = static_cast<int>(normalizedTime * flipbookModule.framePerSecond);
            break;
        }
        }

        // 再生モードに応じてフレームを調整
        switch (flipbookModule.playbackMode) {
            case MeshEffectData::FlipbookPlaybackMode::Once: {
                currentFrame = (std::min)(currentFrame, totalFrames - 1);
                break;
            }
            case MeshEffectData::FlipbookPlaybackMode::Loop: {
                currentFrame %= totalFrames;
                break;
            }
            case MeshEffectData::FlipbookPlaybackMode::PingPong:
            {
                int pingPongFrameCount = totalFrames * 2 - 2; // PingPongのフレーム数
                currentFrame %= pingPongFrameCount;
                if (currentFrame >= totalFrames) {
                    currentFrame = pingPongFrameCount - currentFrame; // 逆再生
                }
                break;
            }
            default: break;
        }

        // UV座標の計算
        int tileX = flipbookModule.tileX;
        int tileY = flipbookModule.tileY;
        float uSize = 1.0f / tileX;
        float vSize = 1.0f / tileY;
        int frameX = currentFrame % tileX;
        int frameY = currentFrame / tileX;
        outUvOffset.x = frameX * uSize; // U offset
        outUvOffset.y = frameY * vSize; // V offset
        outUvTiling.x = uSize;           // U scale
        outUvTiling.y = vSize;           // V scale
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

    // === MeshEffectの更新処理 ===
    if (!meshEffectPool) return;
    auto& meshEffects = meshEffectPool->GetList();

    for (auto& meshEffect : meshEffects) {
        // playOnAwakeが有効で、まだ再生されていない場合は再生する
        if (meshEffect.Main().playOnAwake && !meshEffect.IsPlaying()) {
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

        float normalizedTime = currentTime / meshEffect.Main().duration;

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
                normalizedTime,
                meshEffect.EvaluatedState().buffer.uvOffset,
                meshEffect.EvaluatedState().buffer.uvTiling);
        }

        // === Scrollの更新 === FIX: Flipbookが有効の場合は上手くいかないかも
        if (meshEffect.Scroll().enabled) {
            // ScrollのUVオフセットを更新
            meshEffect.EvaluatedState().buffer.uvOffset.x += meshEffect.Scroll().scrollSpeed.x * scaledDeltaTime;
            meshEffect.EvaluatedState().buffer.uvOffset.y += meshEffect.Scroll().scrollSpeed.y * scaledDeltaTime;
            // UVオフセットを0.0～1.0の範囲にラップ
            meshEffect.EvaluatedState().buffer.uvOffset.x = std::fmod(meshEffect.EvaluatedState().buffer.uvOffset.x, 1.0f);
            meshEffect.EvaluatedState().buffer.uvOffset.y = std::fmod(meshEffect.EvaluatedState().buffer.uvOffset.y, 1.0f);
        }

        // === Waveの更新 ===
        if (meshEffect.Wave().enabled) {
            // Waveの評価
            float waveValue = meshEffect.Wave().amplitudeOverLifetime.Evaluate(normalizedTime);
            meshEffect.EvaluatedState().buffer.uvWaveDirection = meshEffect.Wave().direction;
            meshEffect.EvaluatedState().buffer.uvWaveAmplitude = waveValue;
            meshEffect.EvaluatedState().buffer.uvWaveFrequency = meshEffect.Wave().frequency;
            meshEffect.EvaluatedState().buffer.uvWaveSpeed = meshEffect.Wave().speed;
        }

        // === Gradientの更新 ===
        if (meshEffect.Gradient().enabled) {
            // Gradientの評価
            meshEffect.EvaluatedState().buffer.effectColor = meshEffect.Gradient().color.Evaluate(normalizedTime);
        }

        if (meshEffect.GetTextureResource() == nullptr) {
            std::string texturePath = meshEffect.Renderer().texturePath;
            meshEffect.SetTextureResource(Engine::GetTextureRepository()->GetTextureResource(texturePath));
        }
        if (meshEffect.GetModelResource() == nullptr) {
            std::string modelPath = meshEffect.Renderer().modelPath;
            meshEffect.SetModelResource(Engine::GetModelRepository()->GetModel(modelPath));
        }
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