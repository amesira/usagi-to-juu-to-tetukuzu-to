#pragma once

// プレイヤーエフェクトの種類
enum class PlayerDodgeEffectType {
    DodgeStart,
    DodgeEnd,
};


class PlayerDodgeEffects {
private:

public:
    void Play(PlayerDodgeEffectType type);
};