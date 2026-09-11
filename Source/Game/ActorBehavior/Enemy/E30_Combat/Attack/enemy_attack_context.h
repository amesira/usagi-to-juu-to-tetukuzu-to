// enemy_attack_context.h
// 2026/09/11
#pragma once
#include <DirectXMath.h>
#include "enemy_attack_settings_asset.h"

enum class EnemyAttackPhase { 
    Idle, 
    Windup,     // 予備動作 
    Active,     // 攻撃本体
    Recovery    // 後隙
};

struct EnemyAttackRuntimeState {
    EnemyAttackPhase phase = EnemyAttackPhase::Idle;
    float phaseElapsed = 0.0f;
    float cooldownRemaining = 0.0f;
    DirectX::XMFLOAT3 aimPosition = {};
};

struct EnemyAttackContext {
    const EnemyAttackSettingsAsset* settingsAsset = nullptr;
    EnemyAttackRuntimeState runtimeState;

    const EnemyAttackSettings::Data& settings() const {
        static const EnemyAttackSettings::Data defaults;
        return settingsAsset ? settingsAsset->GetData() : defaults;
    }
};
