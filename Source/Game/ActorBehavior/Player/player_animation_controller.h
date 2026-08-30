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

    /// @brief アニメーションの再生範囲を制限するサブマシーン
    enum class SubMachine {
        Any,
        Default,
        Shotgun,
        DualPistols,

        MAX,
    };

    struct PlayOptions {
        int priority = 0;

        float speed = 1.0f;
        bool loop = true;
        float transitionTime = 0.15f;

        bool waitForCompletion = false; // アニメーションが完了するまで待つ
        bool interruptible = true;      // 割り込み可能かどうか
        bool forceInterrupt = false;    // 強制的に割り込み可能にするかどうか
        bool restart = false;           // 重複呼び出しの際にアニメーションを再スタートするかどうか
    };

private:
    /// @brief アニメーションに対応するアニメーションクリップのインデックスと再生設定を保持する構造体
    struct ClipDefinition {
        int clipIndex = -1;
        SubMachine subMachine = SubMachine::Default;
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

    SubMachine m_currentSubMachine = SubMachine::Default;

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

    /// @brief 指定されたアニメーションに対応するアニメーションクリップのインデックスと再生設定を登録する
    void RegisterClip(
        Animation animation,
        int clipIndex,
        SubMachine subMachine,
        const PlayOptions& defaults);

    // === サブマシーンの管理 ===
    bool EnterSubMachine(SubMachine subMachine);
    void RequestExitSubMachine();
    void ForceSetSubMachine(SubMachine subMachine);

    Animation GetCurrentAnimation() const { return m_currentRequest.animation; }
    bool IsWaitingForCompletion() const { return m_waitingForCompletion; }
    SubMachine GetCurrentSubMachine() const { return m_currentSubMachine; }

private:
    const Request* FindHighestPriorityRequest() const;
    bool IsAvailableInCurrentSubMachine(Animation animation) const;

    /// @brief 指定されたアニメーションリクエストを受け入れ可能かどうかを判定する
    bool CanAccept(const Request& request) const;
    /// @brief 指定されたアニメーションリクエストをAnimationComponentへ適用して再生する
    void Apply(const Request& request);
};
