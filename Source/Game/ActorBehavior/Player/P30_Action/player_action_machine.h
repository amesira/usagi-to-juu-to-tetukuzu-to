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
#include <memory>
#include <unordered_map>

#include "Attack/DualPistols/player_dual_pistols_action.h"
#include "Attack/Shotgun/player_shotgun_action.h"
#include "Attack/WeaponTransform/player_weapon_transform_action.h"

class PlayerContext;
class PlayerInput;

class PlayerActionMachine {
private:
    PlayerDualPistolsAction m_dualPistolsAction;
    PlayerShotgunAction m_shotgunAction;
    PlayerWeaponTransformAction m_weaponTransformAction;

    std::vector<PlayerActionBase*> m_allActions;
    std::vector<PlayerActionBase*> m_activeActions;

    using ActionCategory = PlayerActionBase::ActionCategory;
    using ActionCategoryMask = PlayerActionBase::ActionCategoryMask;

    /// @brief 禁止アクションを表す構造体
    /// 優先度や割り込みに関わらない、最上位の制約として扱う
    /// 禁止マスク > 割り込み可能性 > 優先度
    struct ActionRestriction {
        ActionCategoryMask blockedCategories = 0;
        std::vector<std::string> blockedActionIDs;
    };

    std::unordered_map<ActionCategory, ActionRestriction> m_categoryRestrictions; // カテゴリごとの禁止アクション
    std::unordered_map<std::string, ActionRestriction> m_actionRestrictions;      // アクションIDごとの禁止アクション

public:
    /// @brief 初期化処理
    void Initialize(const PlayerContext& context, const PlayerInput& input);
    /// @brief 更新処理
    void Update(PlayerContext& context, const PlayerInput& input);

private:
    /// @brief 指定されたアクションを開始できるかどうかを判定する
    bool EvaluateStart(PlayerActionBase* action);
    /// @brief 現在のアクションの制約を解決する
    void ResolveRestrictions();

};