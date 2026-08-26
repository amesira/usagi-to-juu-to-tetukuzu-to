//---------------------------------------------------
// File  ：_/Shotgun/player_shotgun_aim.h
// Date  ：2026/08/26
// Author：Miu Kitamura
// 
// ・プレイヤーのショットガンの照準を制御するクラス
//---------------------------------------------------
#pragma once
class PlayerShotgunContext;

class PlayerShotgunAim {
private:
    bool m_isAiming = false;
    int m_locomotionRequestID = -1;

public:
    void EnterAim(PlayerShotgunContext& context);
    void UpdateAim(PlayerShotgunContext& context, float deltaTime);
    void ExitAim(PlayerShotgunContext& context);

};