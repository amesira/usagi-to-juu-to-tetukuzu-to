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

        MAX,
    };

    /// @brief アニメーションの優先度
    enum class Priority : int {
        Locomotion = 10,
        Weapon = 20,
        Airborne = 30,
    };

    struct PlayOptions {
        int priority = 0;

        float speed = 1.0f;
        bool loop = true;

        bool waitForCompletion = false; // アニメーションが完了するまで待つ
        bool interruptible = true;      // 割り込み可能かどうか
        bool forceInterrupt = false;    // 強制的に割り込み可能にするかどうか
        bool restart = false;           // 重複呼び出しの際にアニメーションを再スタートするかどうか
    };

private:
    /// @brief アニメーションに対応するアニメーションクリップのインデックスと再生設定を保持する構造体
    struct ClipDefinition {
        int clipIndex = -1;
        PlayOptions defaults;
    };

    /// @brief アニメーション再生リクエストを保持する構造体
    struct Request {
        Animation animation = Animation::Idle;
        PlayOptions options;
        unsigned long long order = 0;
    };

    AnimationComponent* m_animationComponent = nullptr;

    std::array<ClipDefinition, static_cast<size_t>(Animation::MAX)> m_clipDefinitions = {};
    std::vector<Request> m_frameRequests;

    // === 現在のアニメーション再生状態 ===
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

    void RegisterClip(Animation animation, int clipIndex, const PlayOptions& defaults);

    Animation GetCurrentAnimation() const { return m_currentRequest.animation; }
    bool IsWaitingForCompletion() const { return m_waitingForCompletion; }

private:
    const Request* FindHighestPriorityRequest() const;

    /// @brief 指定されたアニメーションリクエストを受け入れ可能かどうかを判定する
    bool CanAccept(const Request& request) const;
    /// @brief 指定されたアニメーションリクエストをAnimationComponentへ適用して再生する
    void Apply(const Request& request);
};
