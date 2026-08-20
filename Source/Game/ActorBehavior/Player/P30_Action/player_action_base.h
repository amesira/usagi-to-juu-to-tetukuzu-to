//---------------------------------------------------
// File  ：_/Player/P30_Action/player_action_base.h
// Date  ：2026/08/20
// Author：Miu Kitamura
// 
// ・Playerの行動の基底クラス
// ・PlayerActionMachineから呼ばれる
//---------------------------------------------------
#pragma once

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

protected:
    ActionState m_state = ActionState::None;

    // 行動の優先度（数値が大きいほど優先度が高い）
    int m_priority = 0;

    // 割り込み可能かどうか
    // （割り込み可能なアクションは、優先度が高いアクションに置き換えられる）
    bool m_isInterruptible = true;

public:
    virtual ~PlayerActionBase() = default;
    
    /// @brief この行動を開始できるかどうか
    virtual bool CanStart(const PlayerContext& context, const PlayerInput& input) = 0;

    /// @brief 行動開始処理
    virtual void Start(PlayerContext& context, const PlayerInput& input) = 0;
    /// @brief 行動更新処理
    virtual void Update(PlayerContext& context, const PlayerInput& input, float deltaTime) = 0;
    /// @brief 行動終了処理（ActionState::WaitingToFinishに遷移したときに、ActionMachineから呼ばれる）
    virtual void Finish(PlayerContext& context, const PlayerInput& input) = 0;

    /// @brief 行動のアクティブ状態に関わらず、常に呼ばれる更新処理
    virtual void UpdateBackground(PlayerContext& context, const PlayerInput& input, float deltaTime) {

    }

    // === getter/setter ===

    // ActionStateのgetter/setter
    void SetState(ActionState state) { m_state = state; }
    ActionState GetState() const { return m_state; }

    // 優先度のgetter/setter
    void SetPriority(int priority) { m_priority = priority; }
    int GetPriority() const { return m_priority; }

    // 割り込み可能かどうかのgetter/setter
    void SetInterruptible(bool isInterruptible) { m_isInterruptible = isInterruptible; }
    bool IsInterruptible() const { return m_isInterruptible; }

};