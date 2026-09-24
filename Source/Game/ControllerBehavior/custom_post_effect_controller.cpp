//===================================================
// custom_post_effect_controller.cpp
// 
// Author: Miu Kitamura
// Date  : 2026/06/02
//===================================================
#include "custom_post_effect_controller.h"
#include "Engine/Core/scene_interface.h"
#include "Engine/Core/game_object.h"

#include "Engine/Editor/LevelEditor/behavior_detail_view.h"

#include <algorithm>

#include "Engine/Device/mi_fps.h"

#include "game_controller_locator.h"
#include "Utility/debug_ostream.h"

CustomPostEffectController::~CustomPostEffectController()
{
    if (GameControllerLocator::s_customPostEffectController == this) {
        GameControllerLocator::s_customPostEffectController = nullptr;
    }
}

void CustomPostEffectController::Start()
{
    auto* current = GameControllerLocator::s_customPostEffectController;
    if (current && current != this && current->GetEnable()) {
        SetEnable(false);
        return;
    }
    GameControllerLocator::s_customPostEffectController = this;

    m_state.Reset();
    m_state.radialBlur.sampleCount = 4; // デフォルトのサンプル数を設定
    m_state.monoMask.monoColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f); // デフォルトのモノクロ色を設定
}

void CustomPostEffectController::Update()
{
    float deltaTime = FPS_GetUnscaledDeltaTime();
    m_state.horrorNoise.time += deltaTime;

    // 各エフェクトのTweeningタスクを更新し、状態に反映させる
    for (int i = 0; i < static_cast<int>(CustomPostEffectType::MAX); i++) {
        FloatTweenTask& tweenTask = m_changeIntensityTask[i];
        bool isRunning = !tweenTask.IsFinished();

        tweenTask.Update(deltaTime);

        if (isRunning) {
            switch (static_cast<CustomPostEffectType>(i)) {
            case CustomPostEffectType::RadialBlur:
                m_state.radialBlur.strength = tweenTask.m_currentValue;
                break;
            case CustomPostEffectType::MonoMask:
                m_state.monoMask.strength = tweenTask.m_currentValue;
                break;
            case CustomPostEffectType::Mosaic:
                m_state.mosaic.strength = tweenTask.m_currentValue;
                break;
            case CustomPostEffectType::ChromaticAberration:
                m_state.chromaticAberration.strength = tweenTask.m_currentValue;
                break;
            case CustomPostEffectType::Posterize:
                m_state.posterize.strength = tweenTask.m_currentValue;
                break;
            case CustomPostEffectType::HorrorNoise:
                m_state.horrorNoise.strength = tweenTask.m_currentValue;
                break;
            default: break;
            }
        }
    }

    // カスタムポストエフェクト状態の更新
    IScene* scene = GetOwner()->GetScene();
    if (scene) {
        scene->GetPostEffectState() = m_state;
    }
}

void CustomPostEffectController::DrawComponentInspector()
{
    if (BehaviorDetailView::BeginSection(this, "Custom Post Effect Controller")) {
        if (ImGui::TreeNode("Radial Blur")) {
            ImGui::DragInt("Sample Count", &m_state.radialBlur.sampleCount, 1.0f, 1, 64);
            ImGui::DragFloat("Strength", &m_state.radialBlur.strength, 0.01f, 0.0f, 1.0f);

            static float radialIntensity = 0.25f;
            static float radialDuration = 0.2f;
            static float radialHold = 0.1f;

            ImGui::DragFloat("Play Intensity", &radialIntensity, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Play Duration", &radialDuration, 0.01f, 0.0f, 5.0f);
            ImGui::DragFloat("Hold Duration", &radialHold, 0.01f, 0.0f, 5.0f);

            if (ImGui::Button("Play Radial Blur")) {
                PlayEffect(CustomPostEffectType::RadialBlur, radialIntensity, radialDuration, radialHold);
            }

            ImGui::SameLine();

            if (ImGui::Button("Reset Radial Blur")) {
                m_state.radialBlur.strength = 0.0f;
            }

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Mono Mask")) {
            ImGui::ColorEdit4("Mono Color", &m_state.monoMask.monoColor.x);
            ImGui::DragFloat("Strength", &m_state.monoMask.strength, 0.01f, 0.0f, 1.0f);
            ImGui::Text("Mask Texture: %s", m_state.monoMaskTextureSRV ? "Set" : "None");

            static float monoIntensity = 1.0f;
            static float monoDuration = 0.2f;
            static float monoHold = 0.1f;

            ImGui::DragFloat("Play Intensity", &monoIntensity, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Play Duration", &monoDuration, 0.01f, 0.0f, 5.0f);
            ImGui::DragFloat("Hold Duration", &monoHold, 0.01f, 0.0f, 5.0f);

            if (ImGui::Button("Play Mono Mask")) {
                PlayEffect(CustomPostEffectType::MonoMask, monoIntensity, monoDuration, monoHold);
            }

            ImGui::SameLine();

            if (ImGui::Button("Reset Mono Mask")) {
                m_state.monoMask.strength = 0.0f;
            }

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Mosaic")) {
            ImGui::DragFloat("Mosaic Size", &m_state.mosaic.mosaicSize, 1.0f, 1.0f, 256.0f);
            ImGui::DragFloat("Mosaic Strength", &m_state.mosaic.strength, 0.01f, 0.0f, 1.0f);
            if (ImGui::Button("Play Mosaic")) PlayEffect(CustomPostEffectType::Mosaic, 1.0f, 0.2f, 0.1f);
            ImGui::SameLine();
            if (ImGui::Button("Reset Mosaic")) m_state.mosaic.strength = 0.0f;
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Chromatic Aberration")) {
            ImGui::DragFloat("Red Shift Pixels", &m_state.chromaticAberration.redShiftPixels, 0.1f, -64.0f, 64.0f);
            ImGui::DragFloat("Blue Shift Pixels", &m_state.chromaticAberration.blueShiftPixels, 0.1f, -64.0f, 64.0f);
            ImGui::DragFloat("Chromatic Strength", &m_state.chromaticAberration.strength, 0.01f, 0.0f, 1.0f);
            if (ImGui::Button("Play Chromatic Aberration")) PlayEffect(CustomPostEffectType::ChromaticAberration, 1.0f, 0.2f, 0.1f);
            ImGui::SameLine();
            if (ImGui::Button("Reset Chromatic Aberration")) m_state.chromaticAberration.strength = 0.0f;
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Posterize")) {
            ImGui::DragInt("Posterize Levels", &m_state.posterize.levels, 1.0f, 2, 256);
            ImGui::DragFloat("Posterize Strength", &m_state.posterize.strength, 0.01f, 0.0f, 1.0f);
            if (ImGui::Button("Play Posterize")) PlayEffect(CustomPostEffectType::Posterize, 1.0f, 0.2f, 0.1f);
            ImGui::SameLine();
            if (ImGui::Button("Reset Posterize")) m_state.posterize.strength = 0.0f;
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Horror Noise")) {
            ImGui::DragFloat("Noise Min", &m_state.horrorNoise.noiseMin, 0.01f, -1.0f, 1.0f);
            ImGui::DragFloat("Noise Max", &m_state.horrorNoise.noiseMax, 0.01f, -1.0f, 1.0f);
            ImGui::DragFloat("Contrast Pow", &m_state.horrorNoise.contrastPow, 0.01f, 0.001f, 8.0f);
            ImGui::DragFloat("Horror Strength", &m_state.horrorNoise.strength, 0.01f, 0.0f, 1.0f);
            if (ImGui::Button("Play Horror Noise")) PlayEffect(CustomPostEffectType::HorrorNoise, 1.0f, 0.2f, 0.1f);
            ImGui::SameLine();
            if (ImGui::Button("Reset Horror Noise")) m_state.horrorNoise.strength = 0.0f;
            ImGui::TreePop();
        }

        if (ImGui::Button("Reset All Custom Effects")) {
            m_state.Reset();
            m_state.radialBlur.sampleCount = 4;
        }
    }

    BehaviorDetailView::EndSection();
}

// ポストエフェクトの再生
void CustomPostEffectController::PlayEffect(
    CustomPostEffectType effectType, 
    float intensity, float duration, float holdDuration)
{
    int effectIndex = static_cast<int>(effectType);

    FloatTweenTask& tweenTask = m_changeIntensityTask[effectIndex];
    tweenTask.Reset();
    tweenTask.m_duration = duration;
    tweenTask.m_holdDuration = holdDuration;
    tweenTask.m_targetValue = intensity;
    tweenTask.m_endValue = 0.0f; // 終了後は強度0に戻す

    // 開始値は現在のエフェクトの強度にする
    switch (effectType) {
        case CustomPostEffectType::RadialBlur:
            tweenTask.m_startValue = m_state.radialBlur.strength;
            break;
        case CustomPostEffectType::MonoMask:
            tweenTask.m_startValue = m_state.monoMask.strength;
            break;
        case CustomPostEffectType::Mosaic:
            tweenTask.m_startValue = m_state.mosaic.strength;
            break;
        case CustomPostEffectType::ChromaticAberration:
            tweenTask.m_startValue = m_state.chromaticAberration.strength;
            break;
        case CustomPostEffectType::Posterize:
            tweenTask.m_startValue = m_state.posterize.strength;
            break;
        case CustomPostEffectType::HorrorNoise:
            tweenTask.m_startValue = m_state.horrorNoise.strength;
            break;
        default: break;
    }

    tweenTask.Start();
}
