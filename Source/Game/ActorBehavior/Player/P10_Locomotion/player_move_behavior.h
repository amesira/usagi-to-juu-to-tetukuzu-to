//---------------------------------------------------
// player_move_behavior.h
// 
// Author：Miu Kitamura
// Date  ：2026/03/25
//---------------------------------------------------
#ifndef PLAYER_MOVE_BEHAVIOR_H
#define PLAYER_MOVE_BEHAVIOR_H
#include "Engine/Framework/Component/behavior_component.h"

#include "Game/ActorBehavior/Player/P10_Locomotion/player_move_intent.h"
#include "Game/ActorBehavior/Player/P10_Locomotion/player_move_context.h"

class PlayerMoveBehavior : public BehaviorComponent {
private:
    PlayerMoveContext m_context;

public:
    PlayerMoveBehavior() = default;
    ~PlayerMoveBehavior() = default;
    void Start() override;
    void Update() override;
    void DrawComponentInspector() override;

    /// @brief PlayerMoveBehaviorのコンテキストを設定する
    void SetupContext(const class PlayerContext& playerContext);

    /// @brief PlayerMoveBehaviorの更新処理を行う（PlayerBehaviorのUpdate()から呼び出す）
    void UpdateMove(
        const class PlayerContext& context, 
        const class PlayerInput& input, 
        const PlayerMoveIntent& moveIntent, 
        float deltaTime);

private:

};

#endif // PLAYER_MOVE_BEHAVIOR_H
