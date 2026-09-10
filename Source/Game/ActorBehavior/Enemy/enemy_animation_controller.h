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
class AnimationComponent;

/// @brief 敵の状態・移動・Combatからのアニメーション要求をまとめる窓口。
class EnemyAnimationController {
public:
    enum class Animation { Idle, Walk };

    struct Settings {
        float walkStartSpeed = 0.15f;
        float walkStopSpeed = 0.05f;
        float transitionTime = 0.15f;
        float idlePlaybackSpeed = 0.5f;
        float walkPlaybackSpeed = 2.0f;
    };

private:
    Settings m_settings;
    AnimationComponent* m_animationComponent = nullptr;
    int m_idleClipIndex = -1;
    int m_walkClipIndex = -1;
    Animation m_currentAnimation = Animation::Idle;
    bool m_hasCurrentAnimation = false;

public:
    void Initialize(EnemyContext& context);

    void Update(EnemyContext& context);
    void Finalize();

    Animation GetCurrentAnimation() const { return m_currentAnimation; }

private:
    void PlayAnimation(Animation animation);
};
