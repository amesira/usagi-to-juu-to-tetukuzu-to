//---------------------------------------------------
// File  ：_/Shotgun/player_shotgun_charging.h
// Date  ：2026/08/26
// Author：Miu Kitamura
// 
// ・プレイヤーのショットガンのチャージ状態を制御するクラス
//---------------------------------------------------
#pragma once

struct PlayerShotgunContext;

class PlayerShotgunCharging {
private:
    float m_chargeTime = 0.0f;
    bool m_isCharging = false;

public:
    /// @brief チャージを開始する
    void Start(PlayerShotgunContext& context);
    /// @brief チャージ状態を更新する
    void Update(PlayerShotgunContext& context, float deltaTime);
    /// @brief チャージをキャンセルする
    void Cancel(PlayerShotgunContext& context);
    /// @brief チャージ状態を初期状態へ戻す
    void Reset(PlayerShotgunContext& context);

    bool IsCharging() const {
        return m_isCharging;
    }
    bool IsChargeComplete(const PlayerShotgunContext& context) const;

    float GetChargeRate(const PlayerShotgunContext& context) const;
};
