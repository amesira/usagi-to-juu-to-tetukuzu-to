//---------------------------------------------------
// File  ：_/Player/Movement/player_move_rotate.h
// Date  ：2026/08/15
// Author：Miu Kitamura
// 
// ・PlayerMoveBehaviorの回転処理を担当するクラス
//---------------------------------------------------
#pragma once

class PlayerMoveRotate {
public:
    /// @brief PlayerMoveの回転処理を更新する
    void UpdateRotate(struct PlayerMoveContext& context, const struct PlayerMoveIntent& intent, float deltaTime);

};