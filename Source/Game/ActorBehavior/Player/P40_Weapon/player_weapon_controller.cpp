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

#include "Engine/Device/mi_fps.h"

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
    float deltaTime = FPS_GetDeltaTime();
    // 現在の武器の更新
    // リロードなどはActionとして行なうので、ここで何を更新するかは悩み中

    m_recoveryTimer -= deltaTime;
    if (m_recoveryTimer <= 0.0f)
    {
        m_recoveryTimer += m_settingsAsset ? m_settingsAsset->GetData().recoveryTime : 0.2f;
        if (m_ammo < GetMaxAmmo())
        {
            m_ammo++;
        }
    }
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

bool PlayerWeaponController::CanConsume(int cost) const
{
    return m_ammo >= cost;
}

float PlayerWeaponController::TryConsume(int cost)
{
    float consumeCost = cost;
    if (m_ammo < cost) {
        consumeCost = static_cast<float>(m_ammo); // 残りの弾薬が足りない場合は、残りの弾薬を消費する
    }
    m_ammo -= static_cast<int>(consumeCost);
    return consumeCost;
}

int PlayerWeaponController::GetDisplayAmmo() const
{
    return m_ammo;
}

int PlayerWeaponController::GetDisplayCapacity() const
{
    return GetMaxAmmo();
}
