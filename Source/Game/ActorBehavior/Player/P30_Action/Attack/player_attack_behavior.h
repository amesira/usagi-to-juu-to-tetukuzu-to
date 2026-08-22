//---------------------------------------------------
// player_attack_behavior.h
// 
// Author・Miu Kitamura
// Date  ・・026/03/25
//---------------------------------------------------
#ifndef PLAYER_ATTACK_BEHAVIOR_H
#define PLAYER_ATTACK_BEHAVIOR_H

#include "Engine/Component/behavior_component.h"
#include "Game/ActorBehavior/Player/P00_Core/player_context.h"
#include "player_attack_effects.h"

class TransformComponent;
class CameraComponent;
class LightComponent;
class ParticleSystemComponent;

class PlayerAttackBehavior : public BehaviorComponent {
private:
    TransformComponent* m_transform = nullptr;

    // メインカメラの参照
    TransformComponent* m_mainCameraTransform = nullptr;
    CameraComponent* m_mainCamera = nullptr;

    PlayerAttackEffects m_effects;

    // エイム移行前の入力猶予時間
    float m_aimHoldBufferTimer = 0.0f;
    float m_maxAimHoldBufferTime = 0.05f;

    // 攻撃移行前の入力猶予時間
    float m_attackHoldBufferTimer = 0.0f;
    float m_chackSingleAttackBufferTime = 0.15f;

    // チャージ攻撃の蓄積時間
    float m_chargeTimer = 0.0f;
    float m_maxChargeTime = 2.0f;

public:
    PlayerAttackBehavior() = default;
    ~PlayerAttackBehavior() = default;

    void Start() override;
    void Update() override;
    void DrawComponentInspector() override;

    // エイム移行バッファ開始処理
    void StartAimHoldBuffer(PlayerContext& context);
    // エイム移行バッファ更新処理
    void UpdateAimHoldBuffer(PlayerContext& context, float deltaTime, float unscaledDeltaTime);
    // エイム移行バッファ終了判定
    bool IsFinishedAimHoldBuffer() const { 
        return m_aimHoldBufferTimer > m_maxAimHoldBufferTime;
    }

    // エイム開始処理
    void StartAim(PlayerContext& context);
    // エイム更新処理
    void UpdateAim(PlayerContext& context, float deltaTime, float unscaledDeltaTime);
    // エイム終了処理
    void EndAim(PlayerContext& context);

    // 攻撃移行バッファ開始処理
    void StartAttackHoldBuffer(PlayerContext& context);
    // 攻撃移行バッファ更新処理
    void UpdateAttackHoldBuffer(PlayerContext& context, float deltaTime, float unscaledDeltaTime);
    // 単押し攻撃の猶予時間内かどうかの判定
    bool IsSingleAttackBuffer() const {
        return m_attackHoldBufferTimer < m_chackSingleAttackBufferTime;
    }

    // 単発攻撃処理
    void SingleAttack(PlayerContext& context);

    // チャージ開始処理
    void StartCharge(PlayerContext& context);
    // チャージ更新処理
    void UpdateCharge(PlayerContext& context, float deltaTime, float unscaledDeltaTime);
    // チャージ攻撃処理
    void ChargeAttack(PlayerContext& context);

    void SetupChargeEffect(ParticleSystemComponent* chargeEffect) { m_effects.SetupChargeEffect(chargeEffect); }
    void SetupChargeLight(LightComponent* chargeLight) { m_effects.SetupChargeLight(chargeLight); }
    bool IsMovementLocked() const { return m_effects.IsMovementLocked(); }
};

#endif // PLAYER_ATTACK_BEHAVIOR_H
