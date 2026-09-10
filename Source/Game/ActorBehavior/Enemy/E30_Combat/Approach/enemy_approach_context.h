#pragma once
#include "enemy_approach_settings_asset.h"
#include "enemy_approach_navigation.h"
#include "enemy_approach_locomotion.h"
#include "Game/ActorBehavior/Enemy/E00_Core/enemy_context.h"

struct EnemyApproachRuntimeState {
    DirectX::XMFLOAT3 destination = {};
    DirectX::XMFLOAT3 lastPathTargetPosition = {};
    DirectX::XMFLOAT3 lastProgressPosition = {};
    float repathTimer = 0.0f;
    float minRepathTimer = 0.0f;
    float retryTimer = 0.0f;
    float stuckCheckTimer = 0.0f;
    int consecutivePathFailures = 0;
    int consecutiveStuckChecks = 0;
    bool hasReachedDestination = false;
};

struct EnemyApproachContext {
    class EnemyApproachCombat* owner = nullptr;
    const EnemyApproachSettingsAsset* settingsAsset = nullptr;

    TransformComponent* enemyTransform = nullptr;
    
    const EnemyRuntimeState* enemyRuntimeState = nullptr;
    
    EnemyAIWorldController* aiWorld = nullptr;
    EnemyPathFollower* pathFollower = nullptr;
    EnemyLocomotionController* locomotionController = nullptr;
    
    const EnemyAiAgentSettingsAsset* aiAgentSettingsAsset = nullptr;
    
    EnemyApproachRuntimeState runtimeState;
    EnemyApproachNavigation navigation;
    EnemyApproachLocomotion locomotion;

    const EnemyApproachSettings::Data& settings() const {
        static const EnemyApproachSettings::Data defaults;
        return settingsAsset ? settingsAsset->GetData() : defaults;
    }
    const EnemyAiAgentSettings::Data& aiAgentSettings() const {
        static const EnemyAiAgentSettings::Data defaults;
        return aiAgentSettingsAsset ? aiAgentSettingsAsset->GetData() : defaults;
    }
};
