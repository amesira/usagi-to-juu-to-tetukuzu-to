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
    m_definitionTypes.fill(DefinitionType::None);
    m_clipDefinitions = {};
    m_blendTree1DDefinitions = {};
    m_hasCurrentRequest = false;
    m_waitingForCompletion = false;
    m_requestOrder = 0;
    m_currentSubMachine = SubMachine::Default;

    // 再生設定を定義して、アニメーションクリップを登録
    {
        PlayOptions idle;
        idle.priority = static_cast<int>(Priority::Locomotion);
        idle.restart = false;
        idle.transitionTime = 0.15f;
        RegisterClip(Animation::Idle, FindClipIndex(modelComponent, "player_idle.anim.fbx"), SubMachine::Default, idle);

        PlayOptions running = idle;
        running.speed = 2.0f;
        running.restart = false;
        running.transitionTime = 0.15f;
        RegisterClip(Animation::Running, FindClipIndex(modelComponent, "player_running.anim.fbx"), SubMachine::Default, running);

        PlayOptions jump;
        jump.priority = static_cast<int>(Priority::Airborne);
        jump.loop = false;
        jump.waitForCompletion = true;
        jump.interruptible = true;
        m_jumpClips[0] = FindClipIndex(modelComponent, "player_jump_1.anim.fbx");
        m_jumpClips[1] = FindClipIndex(modelComponent, "player_jump_2.anim.fbx");
        RegisterClip(Animation::Jump, m_jumpClips[0], SubMachine::Any, jump);

        PlayOptions falling;
        falling.priority = static_cast<int>(Priority::Airborne);
        RegisterClip(Animation::Falling, FindClipIndex(modelComponent, "player_jump_2.anim.fbx"), SubMachine::Any, falling);

        PlayOptions shotgunIdle;
        shotgunIdle.priority = static_cast<int>(Priority::Weapon);
        shotgunIdle.transitionTime = 0.15f;
        RegisterBlendTree1D(Animation::AimIdle, {
            { FindClipIndex(modelComponent, "player_shotgun_idle.anim.fbx"), 0.0f },
            { FindClipIndex(modelComponent, "player_shotgun_idle_lower.anim.fbx"), -1.0f },
            { FindClipIndex(modelComponent, "player_shotgun_idle_upper.anim.fbx"), 1.0f },
            }, SubMachine::Shotgun, shotgunIdle);
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
        const bool onlyUpdatingBlendTreeParameter = 
            m_hasCurrentRequest &&
            next->animation == m_currentRequest.animation &&
            GetDefinitionType(next->animation) == DefinitionType::BlendTree1D &&
            !next->options.restart;
        if (!onlyUpdatingBlendTreeParameter) {
            HandleAnimationEvent(next->animation);
        }
        Apply(*next);
        return;
    }
}

#pragma region アニメーション再生
void PlayerAnimationController::PlayAnimation(Animation animation)
{
    const size_t index = static_cast<size_t>(animation);
    if (index >= m_clipDefinitions.size()) return;
    if (m_definitionTypes[index] != DefinitionType::Clip) return;
    PlayAnimation(animation, m_clipDefinitions[index].defaults);
}

void PlayerAnimationController::PlayAnimation(Animation animation, const PlayOptions& options)
{
    const size_t index = static_cast<size_t>(animation);
    if (index >= m_clipDefinitions.size()) return;
    if (m_definitionTypes[index] != DefinitionType::Clip) return;
    if (m_clipDefinitions[index].clipIndex < 0) return;

    m_frameRequests.push_back({ animation, options, 0.0f, m_requestOrder++ });
}

void PlayerAnimationController::PlayBlendTree1D(Animation animation, float parameter)
{
    const size_t index = static_cast<size_t>(animation);
    if (index >= m_blendTree1DDefinitions.size()) return;
    if (m_definitionTypes[index] != DefinitionType::BlendTree1D) return;
    PlayBlendTree1D(animation, parameter, m_blendTree1DDefinitions[index].defaults);
}

void PlayerAnimationController::PlayBlendTree1D(
    Animation animation,
    float parameter,
    const PlayOptions& options)
{
    const size_t index = static_cast<size_t>(animation);
    if (index >= m_blendTree1DDefinitions.size()) return;
    if (m_definitionTypes[index] != DefinitionType::BlendTree1D) return;
    if (m_blendTree1DDefinitions[index].nodes.empty()) return;

    m_frameRequests.push_back({ animation, options, parameter, m_requestOrder++ });
}
#pragma endregion

/// @brief 指定されたアニメーションに対応するアニメーションクリップのインデックスと再生設定を登録する
void PlayerAnimationController::RegisterClip(
    Animation animation,
    int clipIndex,
    SubMachine subMachine,
    const PlayOptions& defaults)
{
    const size_t index = static_cast<size_t>(animation);
    if (index >= m_clipDefinitions.size()) return;
    m_clipDefinitions[index] = { clipIndex, subMachine, defaults };
    m_blendTree1DDefinitions[index] = {};
    m_definitionTypes[index] = DefinitionType::Clip;
}

void PlayerAnimationController::RegisterBlendTree1D(
    Animation animation,
    std::vector<AnimationBlendTree1DNode> nodes,
    SubMachine subMachine,
    const PlayOptions& defaults)
{
    const size_t index = static_cast<size_t>(animation);
    if (index >= m_blendTree1DDefinitions.size()) return;

    std::sort(nodes.begin(), nodes.end(),
        [](const AnimationBlendTree1DNode& lhs, const AnimationBlendTree1DNode& rhs) {
            return lhs.threshold < rhs.threshold;
        });

    m_clipDefinitions[index] = {};
    m_blendTree1DDefinitions[index] = { std::move(nodes), subMachine, defaults };
    m_definitionTypes[index] = DefinitionType::BlendTree1D;
}

void PlayerAnimationController::ChangeClip(Animation animation, int clipIndex)
{
    const size_t index = static_cast<size_t>(animation);
    if (index >= m_clipDefinitions.size()) return;
    if (m_definitionTypes[index] != DefinitionType::Clip) return;
    m_clipDefinitions[index].clipIndex = clipIndex;
}

#pragma region サブマシーンの管理
bool PlayerAnimationController::EnterSubMachine(SubMachine subMachine)
{
    if (subMachine == SubMachine::Any || subMachine == SubMachine::MAX) return false;
    if (subMachine == m_currentSubMachine) return true;
    if (m_currentSubMachine != SubMachine::Default) return false;

    ForceSetSubMachine(subMachine);
    return true;
}

void PlayerAnimationController::RequestExitSubMachine()
{
    if (m_currentSubMachine == SubMachine::Default) return;
    ForceSetSubMachine(SubMachine::Default);
}

void PlayerAnimationController::ForceSetSubMachine(SubMachine subMachine)
{
    if (subMachine == SubMachine::Any || subMachine == SubMachine::MAX) return;

    m_currentSubMachine = subMachine;
    m_hasCurrentRequest = false;
    m_waitingForCompletion = false;
}
#pragma endregion

/// @brief 現在のフレームで最も優先度の高いアニメーションリクエストを検索する
const PlayerAnimationController::Request* PlayerAnimationController::FindHighestPriorityRequest() const
{
    const Request* result = nullptr;
    for (const Request& request : m_frameRequests) {
        if (!IsAvailableInCurrentSubMachine(request.animation)) continue;
        if (!result) {
            result = &request;
            continue;
        }

        if (request.options.priority != result->options.priority) {
            if (request.options.priority > result->options.priority) result = &request;
            continue;
        }

        const bool requestIsCurrent =
            m_hasCurrentRequest && request.animation == m_currentRequest.animation;
        const bool resultIsCurrent =
            m_hasCurrentRequest && result->animation == m_currentRequest.animation;
        if (requestIsCurrent != resultIsCurrent) {
            if (requestIsCurrent) result = &request;
            continue;
        }

        if (request.order < result->order) result = &request;
    }
    return result;
}

bool PlayerAnimationController::IsAvailableInCurrentSubMachine(Animation animation) const
{
    const size_t index = static_cast<size_t>(animation);
    if (index >= m_definitionTypes.size()) return false;

    SubMachine owner = SubMachine::Default;
    switch (m_definitionTypes[index]) {
    case DefinitionType::Clip:
        owner = m_clipDefinitions[index].subMachine;
        break;
    case DefinitionType::BlendTree1D:
        owner = m_blendTree1DDefinitions[index].subMachine;
        break;
    default:
        return false;
    }
    return owner == SubMachine::Any || owner == m_currentSubMachine;
}

PlayerAnimationController::DefinitionType PlayerAnimationController::GetDefinitionType(
    Animation animation) const
{
    const size_t index = static_cast<size_t>(animation);
    if (index >= m_definitionTypes.size()) return DefinitionType::None;
    return m_definitionTypes[index];
}

/// @brief 指定されたアニメーションリクエストを受け入れ可能かどうかを判定する
bool PlayerAnimationController::CanAccept(const Request& request) const
{
    // 1. 現在のアニメーションがない場合は、どのリクエストも受け入れ可能
    if (!m_hasCurrentRequest) return true;
    // 2. 同じ1D BlendTreeの場合、再スタートせずParameterを更新するため受け付ける
    if (request.animation == m_currentRequest.animation) {
        if (GetDefinitionType(request.animation) == DefinitionType::BlendTree1D) return true;
        return request.options.restart;
    }
    // 3. 完了待ちでない場合は
    if (!m_waitingForCompletion) return true;
    // 4. 新しいリクエストが強制割り込みの場合
    if (request.options.forceInterrupt) return true;
    // 5. 現在のアニメーションが割り込み可能かどうか
    if (!m_currentRequest.options.interruptible) return false;
    // 6. 新しいリクエストの優先度が高い場合
    return request.options.priority > m_currentRequest.options.priority;
}

/// @brief 指定されたアニメーションリクエストを適用して再生する
void PlayerAnimationController::Apply(const Request& request)
{
    const size_t index = static_cast<size_t>(request.animation);
    switch (m_definitionTypes[index]) {
    case DefinitionType::Clip: {
        m_animationComponent->PlayAnimation(
            m_clipDefinitions[index].clipIndex,
            request.options.speed,
            request.options.loop,
            request.options.restart,
            request.options.transitionTime);
        break;
    }
    case DefinitionType::BlendTree1D: {
        m_animationComponent->PlayBlendTree1D(
            m_blendTree1DDefinitions[index].nodes,
            request.blendTree1DParameter,
            request.options.speed,
            request.options.loop,
            request.options.restart);
        break;
    }
    default:
        return;
    }

    m_currentRequest = request;
    m_hasCurrentRequest = true;
    m_waitingForCompletion = request.options.waitForCompletion;
}

/// @brief 指定されたアニメーションが設定された瞬間に呼び出される
void PlayerAnimationController::HandleAnimationEvent(Animation animation)
{
    switch (animation) {
    case Animation::Jump: 
    {
        m_jumpFlipCount = (m_jumpFlipCount + 1) % 2;
        ChangeClip(Animation::Jump, m_jumpClips[m_jumpFlipCount]);
        break;
    }
    default: break;
    }
}
