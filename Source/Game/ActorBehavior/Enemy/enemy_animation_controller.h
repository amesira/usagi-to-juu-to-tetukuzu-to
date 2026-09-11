//---------------------------------------------------
// File  ：_/Enemy/enemy_animation_controller.h
// Date  ：2026/09/10
// Author：Miu Kitamura
// 
// ・敵の状態・移動・Combatからのアニメーション要求をまとめる窓口
// ・プレイヤーほど複雑にはならないはず
//---------------------------------------------------
#pragma once
#include "Engine/Core/GamePlay/tween_task.h"

class EnemyContext;
class AnimationComponent;

/// @brief 敵の状態・移動・Combatからのアニメーション要求をまとめる窓口。
class EnemyAnimationController {
public:
    enum class Animation { 
        None,
        Idle, 
        Walk,
        JumpPose,
        Slash,
    };

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

    Animation m_currentAnimation = Animation::None;

    Animation m_currentMainAnimation = Animation::None;
    int m_idleClipIndex = -1;
    int m_walkClipIndex = -1;

    bool m_inCombatAnimation = false;
    WaitAndCallbackTask m_combatAnimationStopTask;
    int m_jumpPoseClipIndex = -1;
    int m_slashClipIndex = -1;

public:
    void Initialize(EnemyContext& context);

    void Update(EnemyContext& context);
    void Finalize();

    Animation GetCurrentAnimation() const { return m_currentAnimation; }

    // === 外部からのアニメーション要求 ===
    void PlayCombatAnimation(Animation animation, float playbackSpeed);
    void StopCombatAnimation(float duration = 0.0f);

private:
    void PlayMainAnimation(Animation animation);

};
