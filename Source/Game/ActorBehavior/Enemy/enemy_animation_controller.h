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
