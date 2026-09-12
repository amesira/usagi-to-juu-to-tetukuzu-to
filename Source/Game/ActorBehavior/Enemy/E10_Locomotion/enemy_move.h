//---------------------------------------------------
// File  ：_/E10_Locomotion/enemy_move.h
// Date  ：2026/09/10
// Author：Miu Kitamura
// 
// ・EnemyのMoveIntentを使って実際に移動する処理
//---------------------------------------------------
#pragma once
#include "enemy_move_intent.h"
#include "enemy_move_context.h"

#include "Move/enemy_move_motor.h"
#include "Move/enemy_move_effects.h"
#include "Move/enemy_path_follower.h"

class EnemyContext;
class EnemyMoveSettingsAsset;

class EnemyMove {
private:
    EnemyMoveContext m_context;

    EnemyMoveMotor m_motor;
    EnemyMoveEffects m_effects;
    EnemyPathFollower m_pathFollower;

public:
    void Initialize(EnemyContext& context, const EnemyMoveSettingsAsset* settingsAsset);
    void Finalize();
    void UpdateMove(EnemyContext& context, const EnemyMoveIntent& intent, float deltaTime);

    EnemyPathFollower& GetPathFollower() { return m_pathFollower; }
    const EnemyPathFollower& GetPathFollower() const { return m_pathFollower; }

private:
    bool CheckGrounded();
    bool ResolveNavigationGroundHeight(float& outHeight) const;
    void ApplyHoverHeight(XMFLOAT3& outPosition, float deltaTime);

    void ApplyControlVelocity(XMFLOAT3& outPosition, float deltaTime);
    void ApplyPhysicsVelocity(XMFLOAT3& outPosition, float deltaTime);

};
