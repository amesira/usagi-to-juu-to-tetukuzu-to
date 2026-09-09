//===================================================
// File  ：_/ControllerBehavior/EnemyAI/enemy_ai_controller.cpp
// Date  ：2026/09/09
// Author：Miu Kitamura
//===================================================
#include "enemy_ai_controller.h"

#include "Engine/Core/game_object.h"
#include "Engine/Device/mi_fps.h"
#include "Engine/Editor/LevelEditor/behavior_detail_view.h"

#include "Utility/debug_ostream.h"

#include "Game/ControllerBehavior/game_controller_locator.h"
#include "Engine/engine_service_locator.h"

EnemyAIController::~EnemyAIController()
{
    m_tacticalQuery.Finalize(m_context);
    m_navigation.Finalize(m_context);
    m_metaAI.Finalize(m_context);

    if (GameControllerLocator::s_enemyAIController == this) {
        GameControllerLocator::s_enemyAIController = nullptr;
    }
}

void EnemyAIController::Start()
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

    m_context.settingsAsset = Engine::DataLoader()->GetAsset<EnemyAiSettingsAsset>(
        "asset/Data/enemy_ai_settings.data.json", true);

    m_metaAI.Initialize(m_context);
    m_navigation.Initialize(m_context);
    m_tacticalQuery.Initialize(m_context);

    m_isInitialized = true;
}

void EnemyAIController::Update()
{
    if (!m_isInitialized || GameControllerLocator::s_enemyAIController != this) return;

    const float deltaTime = FPS_GetDeltaTime();

    m_metaAI.Update(m_context, deltaTime);
    m_tacticalQuery.Update(m_context, deltaTime);
}

void EnemyAIController::DrawComponentInspector()
{
    if (BehaviorDetailView::BeginSection(this, "Enemy AI Controller")) {
        ImGui::Text("Initialized: %s", m_isInitialized ? "Yes" : "No");
        ImGui::TextUnformatted("MetaAI / NavigationSystem / TacticalQuerySystem");
    }
    BehaviorDetailView::EndSection();
}
