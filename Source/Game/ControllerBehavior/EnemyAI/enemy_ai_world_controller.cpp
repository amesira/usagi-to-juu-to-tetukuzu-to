//===================================================
// File  ：_/ControllerBehavior/EnemyAI/enemy_ai_controller.cpp
// Date  ：2026/09/09
// Author：Miu Kitamura
//===================================================
#include "enemy_ai_world_controller.h"

#include "Engine/Core/game_object.h"
#include "Engine/Device/mi_fps.h"
#include "Engine/Editor/LevelEditor/behavior_detail_view.h"

#include "Utility/debug_ostream.h"

#include "Game/ControllerBehavior/game_controller_locator.h"
#include "Engine/engine_service_locator.h"
#include "Game/ActorBehavior/Enemy/enemy_behavior.h"
#include "Game/ActorBehavior/Base/health_behavior.h"

EnemyAIWorldController::~EnemyAIWorldController()
{
    m_attackCoordinator.Finalize(m_context);
    m_tacticalQuery.Finalize(m_context);
    m_navigation.Finalize(m_context);
    m_metaAI.Finalize(m_context);

    if (GameControllerLocator::s_enemyAIController == this) {
        GameControllerLocator::s_enemyAIController = nullptr;
    }
}

void EnemyAIWorldController::Start()
{
    if (m_isInitialized) return;
    if (!GetOwner() || !GetOwner()->GetScene()) return;

    // Poolの空きスロット再利用では一時オブジェクトが生成されるため、
    // 登録はコンストラクタではなく配置が確定したStartで行う。
    // 現在のSceneManagerは旧シーンを破棄してから次のシーンを生成する。
    auto* current = GameControllerLocator::s_enemyAIController;
    if (current && current != this && current->GetEnable()) {
        SetEnable(false);
        return;
    }
    GameControllerLocator::s_enemyAIController = this;

    // AIシステムの初期化
    m_context.scene = GetOwner()->GetScene();
    m_context.controller = this;
    m_context.metaAI = &m_metaAI;
    m_context.navigation = &m_navigation;
    m_context.tacticalQuery = &m_tacticalQuery;
    m_context.attackCoordinator = &m_attackCoordinator;

    m_context.settingsAsset = Engine::DataLoader()->GetAsset<EnemyAiWorldSettingsAsset>(
        "asset/Data/enemy_ai_world_settings.data.json", true);

    m_metaAI.Initialize(m_context);
    m_navigation.Initialize(m_context);
    m_tacticalQuery.Initialize(m_context);
    m_attackCoordinator.Initialize(m_context);

    m_isInitialized = true;
}

void EnemyAIWorldController::Update()
{
    if (!m_isInitialized || GameControllerLocator::s_enemyAIController != this) return;

    const float deltaTime = FPS_GetDeltaTime();

    m_metaAI.Update(m_context, deltaTime);
    m_attackCoordinator.Update(m_context, deltaTime);
    m_tacticalQuery.Update(m_context, deltaTime);

    m_navigation.DrawDebugGrid();
}

void EnemyAIWorldController::DrawComponentInspector()
{
    if (BehaviorDetailView::BeginSection(this, "Enemy AI Controller")) {
        ImGui::Text("Initialized: %s", m_isInitialized ? "Yes" : "No");
        ImGui::TextUnformatted("MetaAI / NavigationSystem / TacticalQuerySystem");
        ImGui::Text("Attack: %zu waiting / %zu permitted / %zu active",
            m_attackCoordinator.GetWaitingRequests().size(),
            m_attackCoordinator.GetAttackPermissions().size(),
            m_attackCoordinator.GetCurrentAttackers().size());
        const auto& stats = m_navigation.GetLastSearchStats();
        ImGui::Text("Last Search: %.3f ms / %zu nodes", stats.milliseconds, stats.expandedNodes);
        ImGui::Text("Walkable: %zu evaluations / %zu cache hits",
            stats.walkableEvaluations, stats.walkableCacheHits);
    }
    BehaviorDetailView::EndSection();
}
