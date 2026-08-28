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
    m_currentSubMachine = SubMachine::Default;
    m_subMachineDefaults.fill(Animation::MAX);

    // 再生設定を定義して、アニメーションクリップを登録
    {
        PlayOptions idle;
        idle.priority = static_cast<int>(Priority::Locomotion);
        RegisterClip(Animation::Idle, FindClipIndex(modelComponent, "player_idle.anim.fbx"), SubMachine::Default, idle);

        PlayOptions running = idle;
        running.speed = 2.0f;
        RegisterClip(Animation::Running, FindClipIndex(modelComponent, "player_running.anim.fbx"), SubMachine::Default, running);

        PlayOptions jump;
        jump.priority = static_cast<int>(Priority::Airborne);
        jump.loop = false;
        jump.waitForCompletion = true;
        jump.interruptible = true;
        RegisterClip(Animation::Jump, FindClipIndex(modelComponent, "player_jump_1.anim.fbx"), SubMachine::Any, jump);

        PlayOptions falling;
        falling.priority = static_cast<int>(Priority::Airborne);
        RegisterClip(Animation::Falling, FindClipIndex(modelComponent, "player_jump_2.anim.fbx"), SubMachine::Any, falling);

        PlayOptions shotgunIdle;
        shotgunIdle.priority = static_cast<int>(Priority::Weapon);
        RegisterClip(Animation::AimIdle, FindClipIndex(modelComponent, "player_shotgun_idle.anim.fbx"), SubMachine::Shotgun, shotgunIdle);

        RegisterSubMachineDefault(SubMachine::Default, Animation::Idle);
        RegisterSubMachineDefault(SubMachine::Shotgun, Animation::AimIdle);
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
        return;
    }

    PlayDefaultAnimationIfNeeded();
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
void PlayerAnimationController::RegisterClip(
    Animation animation,
    int clipIndex,
    SubMachine subMachine,
    const PlayOptions& defaults)
{
    const size_t index = static_cast<size_t>(animation);
    if (index >= m_clipDefinitions.size()) return;
    m_clipDefinitions[index] = { clipIndex, subMachine, defaults };
}

void PlayerAnimationController::RegisterSubMachineDefault(
    SubMachine subMachine,
    Animation animation)
{
    const size_t index = static_cast<size_t>(subMachine);
    if (subMachine == SubMachine::Any || index >= m_subMachineDefaults.size()) return;
    m_subMachineDefaults[index] = animation;
}

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
    if (index >= m_clipDefinitions.size()) return false;

    const SubMachine owner = m_clipDefinitions[index].subMachine;
    return owner == SubMachine::Any || owner == m_currentSubMachine;
}

void PlayerAnimationController::PlayDefaultAnimationIfNeeded()
{
    if (m_waitingForCompletion) return;

    const size_t machineIndex = static_cast<size_t>(m_currentSubMachine);
    if (machineIndex >= m_subMachineDefaults.size()) return;

    const Animation animation = m_subMachineDefaults[machineIndex];
    const size_t animationIndex = static_cast<size_t>(animation);
    if (animationIndex >= m_clipDefinitions.size()) return;
    if (!IsAvailableInCurrentSubMachine(animation)) return;

    const ClipDefinition& definition = m_clipDefinitions[animationIndex];
    if (definition.clipIndex < 0) return;
    if (m_hasCurrentRequest && m_currentRequest.animation == animation) return;

    Apply({ animation, definition.defaults, m_requestOrder++ });
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
