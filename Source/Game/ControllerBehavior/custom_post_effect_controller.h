//---------------------------------------------------
// custom_post_effect_controller.h
// 
// Author: Miu Kitamura
// Date  : 2026/06/02
//---------------------------------------------------
#ifndef CUSTOM_POST_EFFECT_CONTROLLER_H
#define CUSTOM_POST_EFFECT_CONTROLLER_H
#include "Engine/Component/behavior_component.h"
#include "Engine/Core/GamePlay/tween_task.h"

#include "Engine/Core/scene_post_effect_state.h"

// カスタムポストエフェクトの種類
enum class CustomPostEffectType {
    RadialBlur,
    MonoMask,
    Mosaic,
    ChromaticAberration,
    Posterize,
    HorrorNoise,

    MAX,
};

class CustomPostEffectController : public BehaviorComponent {
private:
    // ポストエフェクトの状態
    CustomPostEffectState m_state = {};

    // ポストエフェクトの状態をTweeningするタスク
    FloatTweenTask m_changeIntensityTask[static_cast<int>(CustomPostEffectType::MAX)];

public:
    CustomPostEffectController() = default;
    ~CustomPostEffectController();

    void Start() override;
    void Update() override;
    void DrawComponentInspector() override;

    // ポストエフェクトの再生
    void PlayEffect(CustomPostEffectType effectType, float intensity, float duration, float holdDuration);

    // RadialBlurサンプル数設定
    void SetRadialBlur_SampleCount(int count) { m_state.radialBlur.sampleCount = count; }
    // MonoMask色設定
    void SetMonoMask_MonoColor(XMFLOAT4 color) { m_state.monoMask.monoColor = color; }
    // MonoMaskテクスチャ用テクスチャ設定
    void SetMonoMask_Mask(ID3D11ShaderResourceView* srv) { m_state.monoMaskTextureSRV = srv; }
    void SetMosaic_Size(float size) { m_state.mosaic.mosaicSize = size; }
    void SetChromaticAberration_Shift(float redPixels, float bluePixels) {
        m_state.chromaticAberration.redShiftPixels = redPixels;
        m_state.chromaticAberration.blueShiftPixels = bluePixels;
    }
    void SetPosterize_Levels(int levels) { m_state.posterize.levels = levels; }
    void SetHorrorNoise_Parameters(float noiseMin, float noiseMax, float contrastPow) {
        m_state.horrorNoise.noiseMin = noiseMin;
        m_state.horrorNoise.noiseMax = noiseMax;
        m_state.horrorNoise.contrastPow = contrastPow;
    }

};

#endif // CUSTOM_POST_EFFECT_CONTROLLER_H
