//---------------------------------------------------
// File  ：_/E10_Locomotion/enemy_move.h
// Date  ：2026/09/10
// Author：Miu Kitamura
// 
// ・EnemyのMoveIntentを使って実際に移動する処理
//---------------------------------------------------
#pragma once
#include "enemy_move_intent.h"

class EnemyContext;

/// @brief MoveIntentをTransform・Rigidbodyへ反映する。
class EnemyMove {
private:

public:
    void Initialize(EnemyContext& context);
    void Finalize();
    void UpdateMove(EnemyContext& context, const EnemyMoveIntent& intent, float deltaTime);

private:

};
