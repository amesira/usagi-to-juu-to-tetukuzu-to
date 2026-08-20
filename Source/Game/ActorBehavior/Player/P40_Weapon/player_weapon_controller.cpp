//===================================================
// File  ：_/Player/P40_Weapon/player_weapon_controller.cpp
// Date  ：2026/08/21
// Author：Miu Kitamura
// 
// ・プレイヤーの武器の状態や挙動を管理するコントローラー
// ・PlayerBehaviorから呼ばれる
//===================================================
#include "player_weapon_controller.h"

#include "Game/ActorBehavior/Player/P00_Core/player_context.h"
#include "Game/ActorBehavior/Player/P00_Core/player_input.h"

/// @brief 初期化処理
void PlayerWeaponController::Initialize(const PlayerContext& context, const PlayerInput& input)
{
    // 各Weaponの初期化
    
}

/// @brief 更新処理
void PlayerWeaponController::Update(PlayerContext& context, const PlayerInput& input)
{
    // 現在の武器の更新
    // リロードなどはActionとして行なうので、ここで何を更新するかは悩み中
}

/// @brief 武器の切り替えが可能かどうか
bool PlayerWeaponController::CanSwitchWeapon() const
{
    return true; // とりあえず常に切り替え可能にしておく
}

/// @brief 武器を切り替える
void PlayerWeaponController::SwitchWeapon()
{
    
}
