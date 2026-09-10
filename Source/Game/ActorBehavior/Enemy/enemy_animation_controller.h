//---------------------------------------------------
// File  ：_/Enemy/enemy_animation_controller.h
// Date  ：2026/09/10
// Author：Miu Kitamura
// 
// ・敵の状態・移動・Combatからのアニメーション要求をまとめる窓口
// ・プレイヤーほど複雑にはならないはず
//---------------------------------------------------
#pragma once

class EnemyContext;

/// @brief 敵の状態・移動・Combatからのアニメーション要求をまとめる窓口。
class EnemyAnimationController {
public:
    void Initialize(EnemyContext& context);
    void BeginFrame();
    void Update(EnemyContext& context);
    void Finalize();
};
