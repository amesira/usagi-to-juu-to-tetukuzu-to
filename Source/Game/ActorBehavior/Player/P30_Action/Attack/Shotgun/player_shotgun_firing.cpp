//---------------------------------------------------
// File  ：_/Shotgun/player_shotgun_firing.cpp
// Date  ：2026/08/26
// Author：Miu Kitamura
// 
// ・プレイヤーのショットガンの発射処理を制御するクラス
//---------------------------------------------------
#include "player_shotgun_firing.h"
#include "player_shotgun_context.h"

void PlayerShotgunFiring::Fire(PlayerShotgunContext& context, const FireRequest& request)
{
    // 弾の発射処理をここに実装する


    context.effects.PlayEffects(context, PlayerShotgunEffects::EffectsType::Fire);
}
