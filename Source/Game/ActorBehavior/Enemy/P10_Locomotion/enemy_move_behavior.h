#pragma once

#include "enemy_move_intent.h"

class EnemyContext;

/// @brief MoveIntentをTransform・Rigidbodyへ反映する。
class EnemyMoveBehavior {
public:
    void Initialize(EnemyContext& context);
    void Finalize();
    void UpdateMove(EnemyContext& context, const EnemyMoveIntent& intent, float deltaTime);
};
