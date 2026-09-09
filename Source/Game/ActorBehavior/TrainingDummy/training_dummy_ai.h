#pragma once
#include "Game/ControllerBehavior/EnemyAI/enemy_ai_world_context.h"
#include "Game/ControllerBehavior/EnemyAI/enemy_ai_agent_settings_asset.h"

// 追跡判断・再探索・経由点の管理。TransformやRigidbodyは変更しない。
class TrainingDummyAI {
    class TransformComponent* m_transform = nullptr;
    const EnemyAiAgentSettingsAsset* m_settingsAsset = nullptr;
    EnemyAiWorld::NavigationPath m_path;
    size_t m_waypointIndex = 0;
    float m_repathTimer = 0.0f;
    float m_repathInterval = 0.5f;
    float m_moveSpeed = 6.0f;
    float m_reachDistance = 0.2f;
    float m_stopDistance = 5.0f; // 大きなかかしColliderがプレイヤーへ接触する前に停止
    DirectX::XMFLOAT3 m_desiredVelocity = {};
    EnemyAiWorld::PathQueryStatus m_pathStatus = EnemyAiWorld::PathQueryStatus::NotReady;
    bool m_enabled = true;
public:
    void Initialize(class GameObject* owner);
    void Update(float deltaTime);
    void Stop();
    void DrawInspector();
    const DirectX::XMFLOAT3& GetDesiredVelocity() const { return m_desiredVelocity; }
private:
    void UpdatePath(class EnemyAIController& controller, const DirectX::XMFLOAT3& goal);
    void FollowPath(float deltaTime);
};
