//---------------------------------------------------
// File  ：.../ActorBehavior/Player/player_context.h
// Date  ：2026/07/30
// Author：Miu Kitamura
// 
// ・プレイヤーの状態や設定を管理するコンテキストクラス
//---------------------------------------------------
#pragma once
#include <DirectXMath.h>

/// @brief プレイヤーの状態や設定を管理するコンテキストクラス
class PlayerContext {
public:
    class PlayerBehavior* owner = nullptr;
    class TransformComponent* transform = nullptr;

    class PlayerMoveBehavior* moveBehavior = nullptr;
    class PlayerDodgeBehavior* dodgeBehavior = nullptr;
    class PlayerAttackBehavior* attackBehavior = nullptr;

    // FIX: この下にDataAssetで作成したSettingsを入れる予定

};
