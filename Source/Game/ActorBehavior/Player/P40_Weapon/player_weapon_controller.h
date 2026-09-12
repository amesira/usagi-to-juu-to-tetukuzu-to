//---------------------------------------------------
// File  ：_/Player/P40_Weapon/player_weapon_controller.h
// Date  ：2026/08/21
// Author：Miu Kitamura
// 
// ・プレイヤーの武器の状態や挙動を管理するコントローラー
// ・PlayerBehaviorから呼ばれる
//---------------------------------------------------
#pragma once
#include "player_weapon_base.h"
#include <vector>

class PlayerContext;
class PlayerInput;
class PlayerWeaponSettingsAsset;

class PlayerWeaponController {
public:
    enum class WeaponMode {
        DualPistols,
        SlashBurst,
        CombinedShotgun,
    };

    enum class AttackResourceType {
        Shotgun,
        DualPistolsFire,
        DualPistolsSlashBurst,
    };

private:
    // ここにPlayerWeaponBaseの派生クラスのインスタンスを保持する
    
    // 全武器のリスト
    std::vector<PlayerWeaponBase*> m_allWeapons;
    // 現在の武器
    PlayerWeaponBase* m_currentWeapon = nullptr;
    const PlayerWeaponSettingsAsset* m_settingsAsset = nullptr;
    int m_ammo = 0;
    WeaponMode m_weaponMode = WeaponMode::DualPistols;

public:
    /// @brief 初期化処理
    void Initialize(const PlayerWeaponSettingsAsset* settingsAsset);
    /// @brief 更新処理
    void Update(PlayerContext& context, const PlayerInput& input);

    /// @brief 武器の切り替えが可能かどうか
    bool CanSwitchWeapon() const;
    /// @brief 武器を切り替える
    void SwitchWeapon();

    bool CanConsume(AttackResourceType type) const;
    bool TryConsume(AttackResourceType type);

    int GetAmmo() const { return m_ammo; }
    int GetMaxAmmo() const;
    int GetCost(AttackResourceType type) const;

    WeaponMode GetWeaponMode() const { return m_weaponMode; }
    void SetWeaponMode(WeaponMode mode) { m_weaponMode = mode; }

    int GetDisplayAmmo() const;
    int GetDisplayCapacity() const;

};
