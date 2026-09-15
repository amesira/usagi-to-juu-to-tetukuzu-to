//---------------------------------------------------
// File  ：_/E_30_Combat/RePosition/enemy_re_position_combat.h
// Date  ：2026/09/11
// Author：Miu Kitamura
// 
// ・指定された位置へ移動しながら相手を向く。調整先の選択は呼び出し側が担当する
//---------------------------------------------------
#pragma once
#include "Game/ActorBehavior/Enemy/E30_Combat/enemy_combat_base.h"
#include "enemy_re_position_navigation.h"

class EnemyLocomotionController;

struct EnemyRePositionSettings {
    float arrivalDistance = 0.3f;
    float moveSpeedMultiplier = 0.7f;
    float timeout = 3.0f;
    float restartCooldown = 0.5f;
};

/// @brief 指定された位置へ移動しながら相手を向く。調整先の選択は呼び出し側が担当する
class EnemyRePositionCombat : public EnemyCombatBase {
    EnemyRePositionSettings m_settings;
    EnemyRePositionNavigation m_navigation;
    EnemyLocomotionController* m_controller = nullptr;
    int m_requestHandle = -1;
    DirectX::XMFLOAT3 m_destination = {};
    float m_elapsedTime = 0.0f;
    float m_restartCooldown = 0.0f;
    bool m_requested = false;
    bool m_reachedDestination = false;

public:
    EnemyRePositionCombat() : EnemyCombatBase(30, true) {}
    void SetSettings(const EnemyRePositionSettings& settings);
    /// @brief 非実行中に予約する。歩行可能な調整先の選択・再選択は戦闘判断側で行う。
    void RequestPosition(const DirectX::XMFLOAT3& destination);
    bool HasReachedDestination() const { return m_reachedDestination; }
    const DirectX::XMFLOAT3& GetDestination() const { return m_destination; }

    bool CanStart(const EnemyContext& context) const override;
    bool CanContinue(const EnemyContext& context) const override;
    void UpdateBackground(EnemyContext& context, float deltaTime) override;
    void Start(EnemyContext& context) override;
    EnemyCombatStatus Update(EnemyContext& context, float deltaTime) override;
    void Finish(EnemyContext& context) override;
    void Cancel(EnemyContext& context) override;

private:
    bool UpdateLocomotion(const EnemyContext& context);
    void ReleaseResources();
};
