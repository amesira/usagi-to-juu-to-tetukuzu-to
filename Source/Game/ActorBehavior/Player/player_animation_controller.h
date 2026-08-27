#pragma once
#include <array>
#include <vector>

class AnimationComponent;
class PlayerContext;

class PlayerAnimationController {
public:
    enum class Animation {
        Idle,
        Running,
        AimIdle,
        AimRunning,
        Jump,
        Falling,
        Landing,
        Attack,
        Dodge,
        Hit,
        Dead,
        Count,
    };

    enum class Priority : int {
        Locomotion = 10,
        Weapon = 20,
        Airborne = 30,
        Landing = 35,
        Action = 50,
        Dodge = 60,
        Reaction = 80,
        Dead = 100,
    };

    struct PlayOptions {
        int priority = static_cast<int>(Priority::Locomotion);
        float speed = 1.0f;
        bool loop = true;
        bool waitForCompletion = false;
        bool interruptible = true;
        bool forceInterrupt = false;
        bool restart = false;
    };

private:
    struct ClipDefinition {
        int clipIndex = -1;
        PlayOptions defaults;
    };

    struct Request {
        Animation animation = Animation::Idle;
        PlayOptions options;
        unsigned long long order = 0;
    };

    AnimationComponent* m_animationComponent = nullptr;
    std::array<ClipDefinition, static_cast<size_t>(Animation::Count)> m_clipDefinitions = {};
    std::vector<Request> m_frameRequests;

    Request m_currentRequest;
    bool m_hasCurrentRequest = false;
    bool m_waitingForCompletion = false;
    unsigned long long m_requestOrder = 0;

public:
    void Initialize(const PlayerContext& context);
    void BeginFrame();
    void Update();

    void PlayAnimation(Animation animation);
    void PlayAnimation(Animation animation, const PlayOptions& options);
    void SetClip(Animation animation, int clipIndex, const PlayOptions& defaults);

    Animation GetCurrentAnimation() const { return m_currentRequest.animation; }
    bool IsWaitingForCompletion() const { return m_waitingForCompletion; }

private:
    const Request* FindHighestPriorityRequest() const;
    bool CanAccept(const Request& request) const;
    void Apply(const Request& request);
};
