//---------------------------------------------------
// File  ：_/Shotgun/player_shotgun_charging.cpp
// Date  ：2026/08/26
// Author：Miu Kitamura
// 
// ・プレイヤーのショットガンのチャージ状態を制御するクラス
//---------------------------------------------------
#include "player_shotgun_charging.h"

#include "player_shotgun_context.h"

void PlayerShotgunCharging::Start(PlayerShotgunContext& context)
{
    // フェーズとしてはChargingに入ったが、チャージ開始の遅延時間があるため、すぐにはチャージ状態にならない
    m_chargeTime = 0.0f;
}

void PlayerShotgunCharging::Update(PlayerShotgunContext& context, float deltaTime)
{
    m_chargeTime += deltaTime;

    if (!m_isCharging && m_chargeTime >= context.settings().chargeStartDelay) {
        m_isCharging = true;
        context.effects.PlayEffects(context, PlayerShotgunEffects::EffectsType::StartCharge);
    }
}

void PlayerShotgunCharging::Cancel(PlayerShotgunContext& context)
{
    m_isCharging = false;
    m_chargeTime = 0.0f;
}

void PlayerShotgunCharging::Reset(PlayerShotgunContext& context)
{
    m_chargeTime = 0.0f;

    if (m_isCharging) {
        m_isCharging = false;
        context.effects.PlayEffects(context, PlayerShotgunEffects::EffectsType::ResetCharge);

        // FOVがデフォルト値になるのを防ぐため、エイムモードのカメラエフェクトを再度有効化する
        context.aim.SetAimingCameraEffect(context, true);
    }
}

bool PlayerShotgunCharging::IsChargeComplete(const PlayerShotgunContext& context) const
{
    return m_chargeTime >= context.settings().chargeTime;
}

float PlayerShotgunCharging::GetChargeRate(const PlayerShotgunContext& context) const
{
    if (context.settings().chargeTime <= 0.0f) return 1.0f;
    return m_chargeTime / context.settings().chargeTime;
}
