//---------------------------------------------------
// File  ：_/Player/P30_Action/player_action_base.h
// Date  ：2026/08/20
// Author：Miu Kitamura
// 
// ・Playerの行動の基底クラス
// ・PlayerActionMachineから呼ばれる
//---------------------------------------------------
#pragma once
#include <string>

class PlayerContext;
class PlayerInput;

class PlayerActionBase {
public:
    /// @brief 行動の状態
    enum class ActionState {
        None,
        Active,             // 行動中
        WaitingToFinish,    // 行動終了待機中
    };

    /// @brief 行動のカテゴリ
    enum class ActionCategory : std::uint32_t {
        None = 0,
        Attack = 1u << 0,         // 攻撃
        Interaction = 1u << 1,    // アイテムを拾う、話しかける、読むなどのインタラクション
        Reaction = 1u << 2,       // ダメージを受ける、吹き飛ばされる、ノックバックするなどのリアクション
    };
    using ActionCategoryMask = std::uint32_t;
    static constexpr ActionCategoryMask ToMask(ActionCategory category)
    {
        return static_cast<ActionCategoryMask>(category);
    }

protected:
    ActionState m_state = ActionState::None;

    std::string m_actionID;
    ActionCategory m_category = ActionCategory::None;

    // アクションの優先度（数値が大きいほど優先度が高い）
    int m_priority = 0;
    // 割り込み可能かどうか
    // （割り込み可能なアクションは、優先度が高いアクションに置き換えられる）
    bool m_isInterruptible = true;

public:
    PlayerActionBase(std::string actionID, ActionCategory category, int priority, bool isInterruptible = true):
        m_actionID(actionID), m_category(category), m_priority(priority), m_isInterruptible(isInterruptible) {
    }
    virtual ~PlayerActionBase() = default;

    /// @brief このアクションを開始できるかどうか
    virtual bool CanStart(const PlayerContext& context, const PlayerInput& input) = 0;

    /// @brief アクション開始処理
    virtual void Start(PlayerContext& context, const PlayerInput& input) = 0;
    /// @brief アクション更新処理
    virtual void Update(PlayerContext& context, const PlayerInput& input, float deltaTime) = 0;
    /// @brief アクション終了処理（ActionState::WaitingToFinishに遷移したときに、ActionMachineから呼ばれる）
    virtual void Finish(PlayerContext& context, const PlayerInput& input) = 0;

    /// @brief アクションのアクティブ状態に関わらず、常に呼ばれる更新処理
    virtual void UpdateBackground(PlayerContext& context, const PlayerInput& input, float deltaTime) {

    }

    // === getter/setter ===

    void SetState(ActionState state) { m_state = state; }
    ActionState GetState() const { return m_state; }

    const std::string& GetActionID() const { return m_actionID; }
    ActionCategory GetCategory() const { return m_category; }

    int GetPriority() const { return m_priority; }
    bool IsInterruptible() const { return m_isInterruptible; }

};