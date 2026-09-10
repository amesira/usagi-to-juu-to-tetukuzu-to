#include "training_dummy_ai.h"
#include "Engine/Core/game_object.h"
#include "Engine/Component/transform_component.h"
#include "Engine/engine_service_locator.h"
#include "Game/ControllerBehavior/game_controller_locator.h"
#include "Game/ControllerBehavior/EnemyAI/enemy_ai_world_controller.h"
#include "External/ImGui/imgui.h"
#include <algorithm>
#include <cmath>

#include "Utility/debug_renderer.h"

void TrainingDummyAI::Initialize(GameObject* owner)
{
    Stop();
    m_transform = owner->GetComponent<TransformComponent>();
    if (auto* loader = Engine::DataLoader()) {
        m_settingsAsset = loader->GetAsset<EnemyAiAgentSettingsAsset>(
            "asset/Data/training_dummy_ai_agent_settings.data.json", true);
    }
}

void TrainingDummyAI::Stop()
{
    m_path.waypoints.clear();
    m_waypointIndex = 0;
    m_desiredVelocity = {};
    m_repathTimer = 0;
}

void TrainingDummyAI::Update(float deltaTime)
{
    m_desiredVelocity = {};
    auto* controller = Game::EnemyAIWorld();
    if (!m_enabled || !m_transform || !controller || !controller->GetEnable()
        || !controller->IsInitialized() || !controller->GetMetaAI().HasPlayer()) {
        Stop();
        return;
    }
    if (deltaTime <= 0) return;
    const auto goal = controller->GetMetaAI().GetPlayerPosition();
    const auto position = m_transform->GetPosition();
    if (std::hypot(goal.x-position.x, goal.z-position.z) <= m_stopDistance) {
        Stop();
        return;
    }
    m_repathTimer -= deltaTime;
    if (m_repathTimer <= 0) {
        UpdatePath(*controller, goal);
        m_repathTimer = m_repathInterval;
    }
    FollowPath(deltaTime);
}

void TrainingDummyAI::UpdatePath(EnemyAIWorldController& controller, const DirectX::XMFLOAT3& goal)
{
    static const EnemyAiAgentSettings::Data defaults;
    const auto& settings = m_settingsAsset ? m_settingsAsset->GetData() : defaults;
    auto result = controller.FindPath(m_transform->GetPosition(), goal, settings.navigationAgent);
    m_pathStatus = result.status;
    m_path.waypoints.clear();
    m_waypointIndex = 0;
    if (result.status != EnemyAiWorld::PathQueryStatus::Success) return;
    m_path = std::move(result.path);
    // 現在のFindPathは [start, 始点セル中心, ... , goal] を返す。
    // 再探索のたびに現在セルの中心へ引き返さず、次セルへ進む。
    if (m_path.waypoints.size() > 2) m_waypointIndex = 2;
}

void TrainingDummyAI::FollowPath(float deltaTime)
{
    const auto position = m_transform->GetPosition();

    // 経路パスをデバッグ表示
    for (size_t i = 1; i < m_path.waypoints.size(); ++i) {
        DebugRenderer_DrawLine(
            XMFLOAT3(
                m_path.waypoints[i - 1].x,
                position.y + 1.0f,
                m_path.waypoints[i - 1].z),
            XMFLOAT3(
                m_path.waypoints[i].x,
                position.y + 1.0f,
                m_path.waypoints[i].z),
            XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f)
        );
    }

    while (m_waypointIndex < m_path.waypoints.size()) {
        const auto target = m_path.waypoints[m_waypointIndex];
        const float dx = target.x-position.x, dz = target.z-position.z;
        const float distance = std::hypot(dx,dz);
        if (distance <= m_reachDistance) {
            ++m_waypointIndex;
            continue;
        }
        // 経由点を飛び越さない速度に制限。Yは物理の重力・接地へ任せる。
        const float speed = (std::min)(m_moveSpeed, distance/deltaTime);
        m_desiredVelocity = {dx/distance*speed, 0, dz/distance*speed};
        return;
    }
}

void TrainingDummyAI::DrawInspector()
{
    if (!ImGui::TreeNode("Chase AI")) return;
    ImGui::Checkbox("Enabled", &m_enabled);
    ImGui::DragFloat("Move Speed", &m_moveSpeed, 0.1f, 0.0f, 20.0f);
    ImGui::DragFloat("Repath Interval", &m_repathInterval, 0.05f, 0.1f, 5.0f);
    ImGui::DragFloat("Stop Distance", &m_stopDistance, 0.1f, 0.1f, 20.0f);
    ImGui::Text("Path Status: %d", static_cast<int>(m_pathStatus));
    ImGui::Text("Waypoint: %zu / %zu", m_waypointIndex, m_path.waypoints.size());
    ImGui::TreePop();
}
