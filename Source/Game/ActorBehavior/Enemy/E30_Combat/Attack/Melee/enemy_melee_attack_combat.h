//---------------------------------------------------
// File  ：_/E_30_Combat/Attack/Melee/enemy_melee_attack_combat.h
// Date  ：2026/09/11
// Author：Miu Kitamura
// 
// ・近距離攻撃制御クラス
//---------------------------------------------------
#pragma once
#include "Game/ActorBehavior/Enemy/E30_Combat/Attack/enemy_attack_combat.h"
#include "Game/ActorBehavior/Enemy/E10_Locomotion/enemy_locomotion_controller.h"
#include "enemy_melee_attack_slash.h"
#include "enemy_melee_attack_effects.h"

enum class EnemyMeleeAttackPhase {
    Idle, 
    Jump, 
    Slash
};

class EnemyMeleeAttackCombat : public EnemyAttackCombat {
    EnemyMeleeAttackPhase m_phase = EnemyMeleeAttackPhase::Idle;
    float m_elapsedTime = 0.0f;
    bool m_enteredAttackPhase = false;
    EnemyMeleeAttackSlash m_slash;
    EnemyMeleeAttackEffects m_effects;

    EnemyLocomotionController::LocomotionRequest m_locomotionRequest = {};
    int m_locomotionRequestId = -1;

    DirectX::XMFLOAT3 m_jumpStartPosition = {};
    DirectX::XMFLOAT3 m_landingPosition = {};
    DirectX::XMFLOAT3 m_jumpVelocity = {};

protected:
    void BeginWindup(EnemyContext& context) override;
    void EndWindup(EnemyContext& context) override;

    void BeginAttack(EnemyContext& context) override;
    EnemyCombatStatus UpdateAttack(EnemyContext& context, float deltaTime) override;
    void EndAttack(EnemyContext& context) override;

    // 各攻撃段階の処理。
    virtual void BeginJump(EnemyContext& context);
    virtual void UpdateJump(EnemyContext& context, float deltaTime);
    virtual void BeginSlash(EnemyContext& context);
    virtual EnemyCombatStatus UpdateSlash(EnemyContext& context, float deltaTime);
    virtual void ClearAttackEffects(EnemyContext& context);

private:
    void ChangeAttackPhase(EnemyMeleeAttackPhase newPhase);

};
