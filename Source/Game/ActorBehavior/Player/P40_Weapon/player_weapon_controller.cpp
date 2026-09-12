//===================================================
// File  ：_/Player/P40_Weapon/player_weapon_controller.cpp
// Date  ：2026/08/21
// Author：Miu Kitamura
// 
// ・プレイヤーの武器の状態や挙動を管理するコントローラー
// ・PlayerBehaviorから呼ばれる
//===================================================
#include "player_weapon_controller.h"
#include "player_weapon_settings_asset.h"

#include <algorithm>

#include "Game/ActorBehavior/Player/P00_Core/player_context.h"
#include "Game/ActorBehavior/Player/P00_Core/player_input.h"

/// @brief 初期化処理
void PlayerWeaponController::Initialize(const PlayerWeaponSettingsAsset* settingsAsset)
{
    m_settingsAsset = settingsAsset;
    m_ammo = GetMaxAmmo();
    m_weaponMode = WeaponMode::DualPistols;
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
    m_weaponMode = m_weaponMode == WeaponMode::DualPistols
        ? WeaponMode::CombinedShotgun
        : WeaponMode::DualPistols;
}

int PlayerWeaponController::GetMaxAmmo() const
{
    static const PlayerWeaponSettings::Data defaults;
    return (std::max)(m_settingsAsset ? m_settingsAsset->GetData().maxAmmo : defaults.maxAmmo, 1);
}

int PlayerWeaponController::GetCost(AttackResourceType type) const
{
    static const PlayerWeaponSettings::Data defaults;
    const auto& settings = m_settingsAsset ? m_settingsAsset->GetData() : defaults;
    int cost = 1;
    switch (type) {
    case AttackResourceType::Shotgun: cost = settings.shotgunCost; break;
    case AttackResourceType::DualPistolsFire: cost = settings.dualPistolsFireCost; break;
    case AttackResourceType::DualPistolsSlashBurst: cost = settings.dualPistolsSlashBurstCost; break;
    }
    return (std::max)(cost, 1);
}

bool PlayerWeaponController::CanConsume(AttackResourceType type) const
{
    return m_ammo >= GetCost(type);
}

bool PlayerWeaponController::TryConsume(AttackResourceType type)
{
    const int cost = GetCost(type);
    if (m_ammo < cost) return false;
    m_ammo -= cost;
    return true;
}

int PlayerWeaponController::GetDisplayAmmo() const
{
    if (m_weaponMode == WeaponMode::CombinedShotgun) {
        return m_ammo / GetCost(AttackResourceType::Shotgun);
    }
    return m_ammo;
}

int PlayerWeaponController::GetDisplayCapacity() const
{
    if (m_weaponMode == WeaponMode::CombinedShotgun) {
        return GetMaxAmmo() / GetCost(AttackResourceType::Shotgun);
    }
    return GetMaxAmmo();
}
