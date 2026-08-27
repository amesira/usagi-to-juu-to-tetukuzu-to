#include "player_animation_controller.h"

#include <algorithm>

#include "Engine/Component/animation_component.h"
#include "Engine/Core/game_object.h"
#include "Game/ActorBehavior/Player/P00_Core/player_context.h"
#include "Game/ActorBehavior/Player/player_behavior.h"

void PlayerAnimationController::Initialize(const PlayerContext& context)
{
    m_animationComponent = context.owner
        ? context.owner->GetOwner()->GetComponent<AnimationComponent>()
        : nullptr;

    m_frameRequests.clear();
    m_hasCurrentRequest = false;
    m_waitingForCompletion = false;
    m_requestOrder = 0;

    PlayOptions idle;
    idle.priority = static_cast<int>(Priority::Locomotion);
    SetClip(Animation::Idle, 0, idle);

    PlayOptions running = idle;
    running.speed = 2.0f;
    SetClip(Animation::Running, 1, running);
}

void PlayerAnimationController::BeginFrame()
{
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

void PlayerAnimationController::SetClip(Animation animation, int clipIndex, const PlayOptions& defaults)
{
    const size_t index = static_cast<size_t>(animation);
    if (index >= m_clipDefinitions.size()) return;
    m_clipDefinitions[index] = { clipIndex, defaults };
}

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

            const bool lhsIsCurrent = m_hasCurrentRequest && lhs.animation == m_currentRequest.animation;
            const bool rhsIsCurrent = m_hasCurrentRequest && rhs.animation == m_currentRequest.animation;
            if (lhsIsCurrent != rhsIsCurrent) return !lhsIsCurrent;

            return lhs.order > rhs.order;
        });
}

bool PlayerAnimationController::CanAccept(const Request& request) const
{
    if (!m_hasCurrentRequest) return true;
    if (request.animation == m_currentRequest.animation) return request.options.restart;
    if (!m_waitingForCompletion) return true;
    if (request.options.forceInterrupt) return true;
    if (!m_currentRequest.options.interruptible) return false;
    return request.options.priority > m_currentRequest.options.priority;
}

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
