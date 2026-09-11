//---------------------------------------------------
// File  ：_/E_30_Combat/Attack/Melee/enemy_melee_attack_combat.h
// Date  ：2026/09/11
// Author：Miu Kitamura
// 
// ・近距離攻撃制御クラス
//---------------------------------------------------
#pragma once
#include "Game/ActorBehavior/Enemy/E30_Combat/Attack/enemy_attack_combat.h"

enum class EnemyMeleeAttackPhase { Idle, Jump, Slash };

class EnemyMeleeAttackCombat : public EnemyAttackCombat {
    EnemyMeleeAttackPhase m_phase = EnemyMeleeAttackPhase::Idle;
    float m_elapsedTime = 0.0f;
    DirectX::XMFLOAT3 m_jumpStartPosition = {};
    DirectX::XMFLOAT3 m_landingPosition = {};

public:
    EnemyMeleeAttackPhase GetMeleePhase() const { return m_phase; }
    const DirectX::XMFLOAT3& GetJumpStartPosition() const { return m_jumpStartPosition; }
    const DirectX::XMFLOAT3& GetLandingPosition() const { return m_landingPosition; }

protected:
    void BeginAttack(EnemyContext& context) override;
    EnemyCombatStatus UpdateAttack(EnemyContext& context, float deltaTime) override;
    void EndAttack(EnemyContext& context) override;

    // 実処理の接続先。現在は移動・攻撃判定を発生させない。
    virtual void BeginJump(EnemyContext& context);
    virtual void UpdateJump(EnemyContext& context, float deltaTime);
    virtual void BeginSlash(EnemyContext& context);
    virtual void UpdateSlash(EnemyContext& context, float deltaTime);
    virtual void ClearAttackEffects(EnemyContext& context);
};
