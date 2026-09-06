//---------------------------------------------------
// File  ：_/Player/player_animation_controller.h
// Date  ：2026/09
// Author：Miu Kitamura
// 
// ・プレイヤーのアニメーション再生を管理するクラス
// ・ここまで複雑に遷移するのはプレイヤーだけだと仮定して、プレイヤー専用のアニメーションコントローラーとして実装する
//---------------------------------------------------
#pragma once
#include <array>
#include <vector>

#include "Engine/Component/animation_component.h"

class PlayerContext;

class PlayerAnimationController {
public:
    enum class Animation {
        Idle,
        Running,
        Jump,
        Falling,

        ShotgunAiming,
        DualPistolsRapidFire,
        DualPistolsSlashBurst1,
        DualPistolsSlashBurst2,
        DualPistolsSlashBurst3,

        MAX,
    };

    /// @brief ベースアニメーションへ重ねるアニメーションレイヤー
    enum class AnimationLayer {
        ShotgunAimVertical,
        DualPistolsRapidFireVertical,

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
        float transitionTime = 0.15f;   // 遷移時間（秒）

        bool waitForCompletion = false; // アニメーションが完了するまで待つ
        bool interruptible = true;      // 割り込み可能かどうか
        bool forceInterrupt = false;    // 強制的に割り込み可能にするかどうか
        bool restart = false;           // 重複呼び出しの際にアニメーションを再スタートするかどうか
    };

private:
    enum class DefinitionType {
        None,
        Clip,
        BlendTree1D,
        BlendTree2D,
    };

    /// @brief アニメーションに対応するアニメーションクリップのインデックスと再生設定を保持する構造体
    struct ClipDefinition {
        int clipIndex = -1;
        SubMachine subMachine = SubMachine::Default;
        PlayOptions defaults;
    };

    /// @brief アニメーションに対応する1D BlendTreeのノードと再生設定を保持する構造体
    struct BlendTree1DDefinition {
        std::vector<AnimationBlendTree1DNode> nodes;
        SubMachine subMachine = SubMachine::Default;
        PlayOptions defaults;
    };

    /// @brief アニメーションに対応する2D BlendTreeのノードと再生設定を保持する構造体
    struct BlendTree2DDefinition {
        std::vector<AnimationBlendTree2DNode> nodes;
        SubMachine subMachine = SubMachine::Default;
        PlayOptions defaults;
    };

    struct LayerDefinition {
        DefinitionType type = DefinitionType::None;
        size_t componentLayerIndex = static_cast<size_t>(-1);
        int clipIndex = -1;
        std::vector<AnimationBlendTree1DNode> blendTree1DNodes;
        std::vector<AnimationBlendTree2DNode> blendTree2DNodes;
        SubMachine subMachine = SubMachine::Default;
        PlayOptions defaults;
        float defaultWeight = 1.0f;
    };

    /// @brief アニメーション再生リクエストを保持する構造体
    struct Request {
        Animation animation = Animation::Idle;
        PlayOptions options;

        float blendTree1DParameter = 0.0f;
        DirectX::XMFLOAT2 blendTree2DParameter = {};

        unsigned long long order = 0;
    };

    /// @brief アニメーションレイヤー再生リクエストを保持する構造体
    struct LayerRequest {
        AnimationLayer layer = AnimationLayer::ShotgunAimVertical;
        PlayOptions options;

        float blendTree1DParameter = 0.0f;
        DirectX::XMFLOAT2 blendTree2DParameter = {};

        float weight = 1.0f;
        unsigned long long order = 0;
    };

    AnimationComponent* m_animationComponent = nullptr;

    // === ベースアニメーション ===
    std::array<DefinitionType, static_cast<size_t>(Animation::MAX)> m_definitionTypes = {};

    std::array<ClipDefinition, static_cast<size_t>(Animation::MAX)> m_clipDefinitions = {};
    std::array<BlendTree1DDefinition, static_cast<size_t>(Animation::MAX)> m_blendTree1DDefinitions = {};
    std::array<BlendTree2DDefinition, static_cast<size_t>(Animation::MAX)> m_blendTree2DDefinitions = {};

    std::vector<Request> m_frameRequests;

    // === アニメーションレイヤー ===
    std::array<LayerDefinition, static_cast<size_t>(AnimationLayer::MAX)> m_layerDefinitions = {};
    
    std::vector<LayerRequest> m_frameLayerRequests;

    SubMachine m_currentSubMachine = SubMachine::Default;

    // === 現在のアニメーション再生状態 ===
    Request m_currentRequest;
    bool m_hasCurrentRequest = false;
    bool m_waitingForCompletion = false;
    unsigned long long m_requestOrder = 0;

    // === アニメーション固有の状態 ===
    int m_jumpClips[2] = { -1, -1 };
    int m_jumpFlipCount = 0; // ジャンプ中のフリップ回数

public:
    void Initialize(const PlayerContext& context);
    void BeginFrame();
    void Update();

    // === アニメーション再生 ===
    void PlayAnimation(Animation animation);
    void PlayAnimation(Animation animation, const PlayOptions& options);
    void PlayBlendTree1D(Animation animation, float parameter);
    void PlayBlendTree1D(Animation animation, float parameter, const PlayOptions& options);
    void PlayBlendTree2D(Animation animation, const DirectX::XMFLOAT2& parameter);
    void PlayBlendTree2D(
        Animation animation,
        const DirectX::XMFLOAT2& parameter,
        const PlayOptions& options);

    // === アニメーションレイヤー再生 ===
    void PlayLayerAnimation(AnimationLayer layer, float weight = 1.0f);
    void PlayLayerAnimation(AnimationLayer layer, float weight, const PlayOptions& options);
    void PlayLayerBlendTree1D(AnimationLayer layer, float parameter, float weight = 1.0f);
    void PlayLayerBlendTree1D(
        AnimationLayer layer,
        float parameter,
        float weight,
        const PlayOptions& options);
    void PlayLayerBlendTree2D(
        AnimationLayer layer,
        const DirectX::XMFLOAT2& parameter,
        float weight = 1.0f);
    void PlayLayerBlendTree2D(
        AnimationLayer layer,
        const DirectX::XMFLOAT2& parameter,
        float weight,
        const PlayOptions& options);

    /// @brief 指定されたアニメーションに対応するアニメーションクリップのインデックスと再生設定を登録する
    void RegisterClip(
        Animation animation,
        int clipIndex,
        SubMachine subMachine,
        const PlayOptions& defaults);
    void RegisterBlendTree1D(
        Animation animation,
        std::vector<AnimationBlendTree1DNode> nodes,
        SubMachine subMachine,
        const PlayOptions& defaults);
    void RegisterBlendTree2D(
        Animation animation,
        std::vector<AnimationBlendTree2DNode> nodes,
        SubMachine subMachine,
        const PlayOptions& defaults);
    void RegisterLayerClip(
        AnimationLayer layer,
        int clipIndex,
        const AnimationBoneMask& mask,
        SubMachine subMachine,
        const PlayOptions& defaults,
        float defaultWeight = 1.0f);
    void RegisterLayerBlendTree1D(
        AnimationLayer layer,
        std::vector<AnimationBlendTree1DNode> nodes,
        const AnimationBoneMask& mask,
        SubMachine subMachine,
        const PlayOptions& defaults,
        float defaultWeight = 1.0f);
    void RegisterLayerBlendTree2D(
        AnimationLayer layer,
        std::vector<AnimationBlendTree2DNode> nodes,
        const AnimationBoneMask& mask,
        SubMachine subMachine,
        const PlayOptions& defaults,
        float defaultWeight = 1.0f);
    void ChangeClip(Animation animation, int clipIndex);

    // === サブマシーンの管理 ===
    bool EnterSubMachine(SubMachine subMachine);
    void RequestExitSubMachine();
    void ForceSetSubMachine(SubMachine subMachine);

    Animation GetCurrentAnimation() const { return m_currentRequest.animation; }
    bool IsWaitingForCompletion() const { return m_waitingForCompletion; }
    SubMachine GetCurrentSubMachine() const { return m_currentSubMachine; }

    PlayOptions GetDefaultPlayOptions(Animation animation) const;
    PlayOptions GetDefaultPlayOptions(AnimationLayer layer) const;

private:
    const Request* FindHighestPriorityRequest() const;
    const LayerRequest* FindHighestPriorityLayerRequest(AnimationLayer layer) const;
    DefinitionType GetDefinitionType(Animation animation) const;
    bool IsAvailableInCurrentSubMachine(Animation animation) const;

    /// @brief 指定されたアニメーションリクエストを受け入れ可能かどうかを判定する
    bool CanAccept(const Request& request) const;
    /// @brief 指定されたアニメーションリクエストをAnimationComponentへ適用して再生する
    void Apply(const Request& request);
    void UpdateBaseAnimation();
    void UpdateAnimationLayers();
    void ApplyLayer(const LayerRequest& request);

    /// @brief アニメーションが設定された瞬間に呼び出される
    void HandleAnimationEvent(Animation animation);
};
