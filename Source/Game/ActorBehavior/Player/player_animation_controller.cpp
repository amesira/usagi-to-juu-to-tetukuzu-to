#include "player_animation_controller.h"

#include <algorithm>
#include <filesystem>

#include "Engine/Component/animation_component.h"
#include "Engine/Component/model_component.h"
#include "Engine/Core/game_object.h"
#include "Game/ActorBehavior/Player/P00_Core/player_context.h"
#include "Game/ActorBehavior/Player/player_behavior.h"

namespace {
    /// @brief 指定されたModelComponentのアニメーションクリップの中から、指定されたファイル名に一致するクリップのインデックスを検索する
    int FindClipIndex(const ModelComponent* modelComponent, const std::filesystem::path& fileName)
    {
        if (!modelComponent || !modelComponent->GetModelResource()) return -1;

        const auto& clips = modelComponent->GetModelResource()->animationClips;
        const auto it = std::find_if(clips.begin(), clips.end(), [&fileName](const AnimationClip& clip) {
            return clip.filePath.filename() == fileName;
        });
        return it == clips.end() ? -1 : static_cast<int>(std::distance(clips.begin(), it));
    }
}

void PlayerAnimationController::Initialize(const PlayerContext& context)
{
    GameObject* player = context.owner ? context.owner->GetOwner() : nullptr;
    if (!player) return;

    m_animationComponent = player->GetComponent<AnimationComponent>();
    ModelComponent* modelComponent = player->GetComponent<ModelComponent>();

    m_frameRequests.clear();
    m_hasCurrentRequest = false;
    m_waitingForCompletion = false;
    m_requestOrder = 0;

    // 再生設定を定義して、アニメーションクリップを登録
    {
        PlayOptions idle;
        idle.priority = static_cast<int>(Priority::Locomotion);
        RegisterClip(Animation::Idle, FindClipIndex(modelComponent, "player_idle.anim.fbx"), idle);

        PlayOptions running = idle;
        running.speed = 2.0f;
        RegisterClip(Animation::Running, FindClipIndex(modelComponent, "player_running.anim.fbx"), running);

        PlayOptions jump;
        jump.priority = static_cast<int>(Priority::Airborne);
        jump.loop = false;
        jump.waitForCompletion = true;
        jump.interruptible = true;
        RegisterClip(Animation::Jump, FindClipIndex(modelComponent, "player_jump_1.anim.fbx"), jump);

        PlayOptions falling;
        falling.priority = static_cast<int>(Priority::Airborne);
        RegisterClip(Animation::Falling, FindClipIndex(modelComponent, "player_jump_2.anim.fbx"), falling);

        PlayOptions shotgunIdle;
        shotgunIdle.priority = static_cast<int>(Priority::Weapon);
        RegisterClip(Animation::AimIdle, FindClipIndex(modelComponent, "player_shotgun_idle.anim.fbx"), shotgunIdle);
    }
}

void PlayerAnimationController::BeginFrame()
{
    // 前フレームのリクエストをクリア
    m_frameRequests.clear();
}

void PlayerAnimationController::Update()
{
    if (!m_animationComponent) return;

    if (m_waitingForCompletion && m_animationComponent->IsFinished()) {
        m_waitingForCompletion = false;
        m_hasCurrentRequest = false;
    }

    const Request* next = FindHighestPriorityRequest();
    if (next && CanAccept(*next)) {
        Apply(*next);
    }
}

#pragma region アニメーション再生
void PlayerAnimationController::PlayAnimation(Animation animation)
{
    const size_t index = static_cast<size_t>(animation);
    if (index >= m_clipDefinitions.size()) return;
    PlayAnimation(animation, m_clipDefinitions[index].defaults);
}

void PlayerAnimationController::PlayAnimation(Animation animation, const PlayOptions& options)
{
    const size_t index = static_cast<size_t>(animation);
    if (index >= m_clipDefinitions.size()) return;
    if (m_clipDefinitions[index].clipIndex < 0) return;

    m_frameRequests.push_back({ animation, options, m_requestOrder++ });
}
#pragma endregion

/// @brief 指定されたアニメーションに対応するアニメーションクリップのインデックスと再生設定を登録する
void PlayerAnimationController::RegisterClip(Animation animation, int clipIndex, const PlayOptions& defaults)
{
    const size_t index = static_cast<size_t>(animation);
    if (index >= m_clipDefinitions.size()) return;
    m_clipDefinitions[index] = { clipIndex, defaults };
}

/// @brief 現在のフレームで最も優先度の高いアニメーションリクエストを検索する
const PlayerAnimationController::Request* PlayerAnimationController::FindHighestPriorityRequest() const
{
    if (m_frameRequests.empty()) return nullptr;

    return &*std::max_element(
        m_frameRequests.begin(),
        m_frameRequests.end(),
        [this](const Request& lhs, const Request& rhs)
        {
            if (lhs.options.priority != rhs.options.priority) {
                return lhs.options.priority < rhs.options.priority;
            }

            // 同じ優先度の場合、現在再生中のアニメーションを優先する
            const bool lhsIsCurrent = m_hasCurrentRequest && lhs.animation == m_currentRequest.animation;
            const bool rhsIsCurrent = m_hasCurrentRequest && rhs.animation == m_currentRequest.animation;
            if (lhsIsCurrent != rhsIsCurrent) return !lhsIsCurrent;

            return lhs.order > rhs.order;
        });
}

/// @brief 指定されたアニメーションリクエストを受け入れ可能かどうかを判定する
bool PlayerAnimationController::CanAccept(const Request& request) const
{
    // 現在のアニメーションがない場合は、どのリクエストも受け入れ可能
    if (!m_hasCurrentRequest) return true;
    // 現在のアニメーションと同じアニメーションの場合は、restartオプションに従う
    if (request.animation == m_currentRequest.animation) return request.options.restart;
    // 現在のアニメーションが完了待ちでない場合は、どのリクエストも受け入れ可能
    if (!m_waitingForCompletion) return true;
    // 現在のアニメーションが完了待ちで、かつ新しいリクエストが強制割り込みの場合は受け入れ可能
    if (request.options.forceInterrupt) return true;
    // 現在のアニメーションが割り込み可能かどうかを判定する
    if (!m_currentRequest.options.interruptible) return false;
    // 現在のアニメーションよりも新しいリクエストの優先度が高い場合は受け入れ可能
    return request.options.priority > m_currentRequest.options.priority;
}

/// @brief 指定されたアニメーションリクエストを適用して再生する
void PlayerAnimationController::Apply(const Request& request)
{
    const size_t index = static_cast<size_t>(request.animation);
    const int clipIndex = m_clipDefinitions[index].clipIndex;

    m_animationComponent->PlayAnimation(
        clipIndex,
        request.options.speed,
        request.options.loop,
        request.options.restart);

    m_currentRequest = request;
    m_hasCurrentRequest = true;
    m_waitingForCompletion = request.options.waitForCompletion;
}
