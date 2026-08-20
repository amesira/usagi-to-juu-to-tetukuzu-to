//---------------------------------------------------
// File  ：_/Player/P30_Action/player_action_machine.h
// Date  ：2026/08/20
// Author：Miu Kitamura
// 
// ・Playerの行動を管理するマシン
// ・PlayerBehaviorから呼ばれる
//---------------------------------------------------
#pragma once
#include "player_action_base.h"
#include <vector>

class PlayerContext;
class PlayerInput;

class PlayerActionMachine {
private:
    // ここにPlayerActionBaseの派生クラスのインスタンスを保持する

    // 全アクションのリスト
    std::vector<PlayerActionBase*> m_allActions;
    // 現在の行動
    PlayerActionBase* m_currentAction = nullptr;

public:
    /// @brief 初期化処理
    void Initialize(const PlayerContext& context, const PlayerInput& input);
    /// @brief 更新処理
    void Update(PlayerContext& context, const PlayerInput& input);

};