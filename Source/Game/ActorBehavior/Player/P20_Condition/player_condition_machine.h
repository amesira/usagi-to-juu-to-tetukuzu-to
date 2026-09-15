#pragma once


// プレイヤーの状態
enum class PlayerState {
    Idle,
    Move,
    Dodge,
    Stunned,
};

class PlayerContext;
class PlayerInput;

class PlayerConditionMachine {
private:

public:
    void Update(PlayerContext& context, const PlayerInput& input);

};