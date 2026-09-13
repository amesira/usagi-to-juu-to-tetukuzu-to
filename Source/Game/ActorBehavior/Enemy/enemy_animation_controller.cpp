//===================================================
// File  ：_/Enemy/enemy_animation_controller.cpp
// Date  ：2026/09/10
// Author：Miu Kitamura
//===================================================
#include "enemy_animation_controller.h"

#include "Game/ActorBehavior/Enemy/E00_Core/enemy_context.h"
#include "Game/ActorBehavior/Enemy/E20_Condition/enemy_condition_machine.h"
#include "Engine/Component/animation_component.h"
#include "Engine/Component/model_component.h"
#include "Engine/Component/transform_component.h"
#include "Engine/Core/game_object.h"
#include "Engine/Graphics/model_repository.h"
#include "Engine/engine_service_locator.h"
#include <cmath>
#include <filesystem>

#include "Engine/Device/mi_fps.h"

namespace {
    int FindOrLoadClip(ModelResource* model, const std::filesystem::path& path)
    {
        // 複数の敵が共有するModelResourceへ、同じクリップを重複登録しない。
        for (size_t i = 0; i < model->animationClips.size(); ++i) {
            if (model->animationClips[i].filePath == path) return static_cast<int>(i);
        }
        auto* repository = Engine::ModelRepository();
        return repository ? repository->LoadAnimation(model, path) : -1;
    }
}

void EnemyAnimationController::Initialize(EnemyContext& context)
{
    auto* owner = context.transform ? context.transform->GetOwner() : nullptr;
    if (!owner) return;

    m_animationComponent = owner->GetComponent<AnimationComponent>();
    auto* modelComponent = owner->GetComponent<ModelComponent>();
    auto* model = modelComponent ? modelComponent->GetModelResource() : nullptr;
    if (!m_animationComponent || !model) return;

    const std::string prefix = m_modelB ? "asset/Model/enemy_b_" : "asset/Model/enemy_a_";
    m_idleClipIndex = FindOrLoadClip(model, prefix + "idle.anim.fbx");
    m_walkClipIndex = FindOrLoadClip(model, prefix + "walk.anim.fbx");
    if (m_modelB) {
        m_shotClipIndex = FindOrLoadClip(model, prefix + "shot.anim.fbx");
    }
    else {
        m_jumpPoseClipIndex = FindOrLoadClip(model, prefix + "jump_pose.anim.fbx");
        m_slashClipIndex = FindOrLoadClip(model, prefix + "slash.anim.fbx");
    }

    PlayMainAnimation(Animation::Idle);
}

void EnemyAnimationController::Update(EnemyContext& context)
{
    if (!m_animationComponent) return;

    if (context.conditionMachine) {
        const auto condition = context.conditionMachine->GetCurrentCondition();
        if (condition == EnemyCondition::Stun || condition == EnemyCondition::Dead) {
            return;
        }
    }

    // 速度に応じてメインアニメーションを切り替える
    const auto velocity = context.runtimeState.controlVelocity;
    const float speed = std::hypot(velocity.x, velocity.z);
    Animation next = m_currentAnimation;
    if (m_currentAnimation == Animation::Walk) {
        if (speed <= m_settings.walkStopSpeed) next = Animation::Idle;
    }
    else if (speed >= m_settings.walkStartSpeed) {
        next = Animation::Walk;
    }
    PlayMainAnimation(next);

    // Combatアニメーション停止の待機タスクを更新する
    float deltaTime = FPS_GetDeltaTime();
    m_combatAnimationStopTask.Update(deltaTime);
}

void EnemyAnimationController::PlayMainAnimation(Animation animation)
{
    if (!m_animationComponent) return;
    if (m_inCombatAnimation) return;
    if (animation == m_currentAnimation) return;

    switch (animation) {
    case Animation::Walk:{
        m_animationComponent->PlayAnimation(m_walkClipIndex, 
            m_settings.walkPlaybackSpeed, true, false, m_settings.transitionTime);

        break;
    }
    case Animation::Idle:{
        m_animationComponent->PlayAnimation(m_idleClipIndex, 
            m_settings.idlePlaybackSpeed, true, false, m_settings.transitionTime);
        break;
    }
    default: return;
    }

    m_currentMainAnimation = animation;
    m_currentAnimation = animation;
}

void EnemyAnimationController::PlayCombatAnimation(Animation animation, float playbackSpeed)
{
    if (!m_animationComponent) return;
    m_combatAnimationStopTask.Finish();
    m_inCombatAnimation = true;

    switch (animation) {
        case Animation::Shot: {
            if (m_shotClipIndex < 0) { m_inCombatAnimation = false; return; }
            m_animationComponent->PlayAnimation(m_shotClipIndex, playbackSpeed, false, true, 0.05f);
            break;
        }
        case Animation::JumpPose: {
            // ジャンプはループ再生
            m_animationComponent->PlayAnimation(m_jumpPoseClipIndex, playbackSpeed, true, true, 0.1f);
            break;
        }
        case Animation::Slash: {
            m_animationComponent->PlayAnimation(m_slashClipIndex, playbackSpeed, false, true, 0.1f);
            break;
        }
        default: return;

    }

    m_currentAnimation = animation;
}

void EnemyAnimationController::StopCombatAnimation(float duration)
{
    if (duration <= 0.0f) {
        m_combatAnimationStopTask.Finish();
        m_inCombatAnimation = false;
        PlayMainAnimation(m_currentMainAnimation);
        return;
    }
    m_combatAnimationStopTask.m_waitDuration = duration;
    m_combatAnimationStopTask.m_callback = [this]() {
        // Combatアニメーションが終了したら、メインアニメーションに戻す
        m_inCombatAnimation = false;
        PlayMainAnimation(m_currentMainAnimation);
        };
    m_combatAnimationStopTask.Start();
}

void EnemyAnimationController::Finalize()
{
    m_animationComponent = nullptr;
    m_idleClipIndex = -1;
    m_walkClipIndex = -1;
    m_currentAnimation = Animation::Idle;
}
