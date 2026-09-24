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
    m_isCharging = false;
    m_chargeCompleteEffectPlayed = false;
}

void PlayerShotgunCharging::Update(PlayerShotgunContext& context, float deltaTime)
{
    m_chargeTime += deltaTime;

    if (!m_isCharging && m_chargeTime >= context.settings().chargeStartDelay) {
        m_isCharging = true;
        context.effects.PlayEffects(context, PlayerShotgunEffects::EffectsType::StartCharge);
    }

    if (IsChargeComplete(context) && !m_chargeCompleteEffectPlayed) {
        m_chargeCompleteEffectPlayed = true;
        context.effects.PlayEffects(context, PlayerShotgunEffects::EffectsType::ChargeComplete);
    }
}

void PlayerShotgunCharging::Cancel(PlayerShotgunContext& context)
{
    m_isCharging = false;
    m_chargeTime = 0.0f;
    m_chargeCompleteEffectPlayed = false;
}

void PlayerShotgunCharging::Reset(PlayerShotgunContext& context)
{
    // ChargeRateを後に取得するため、m_chargeTimeはリセットせずに残す

    if (m_isCharging) {
        m_isCharging = false;
        //context.effects.PlayEffects(context, PlayerShotgunEffects::EffectsType::ResetCharge);

        // FOVがデフォルト値になるのを防ぐため、エイムモードのカメラエフェクトを再度有効化する
        context.aim.SetAimingCameraSetting(context, true);
    }
}

bool PlayerShotgunCharging::IsChargeComplete(const PlayerShotgunContext& context) const
{
    return m_chargeTime >= context.settings().chargeTime;
}

float PlayerShotgunCharging::GetChargeRate(const PlayerShotgunContext& context) const
{
    if (context.settings().chargeTime <= 0.0f) return 1.0f;
    // 遅延期間を脱していない場合はチャージ率は0.0fとして、通常弾の発射を行う
    if (m_chargeTime <= context.settings().chargeStartDelay) return 0.0f;
    return m_chargeTime / context.settings().chargeTime;
}
