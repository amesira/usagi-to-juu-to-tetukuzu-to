//---------------------------------------------------
// File  ：_/ControllerBehavior/EnemyAI/enemy_ai_world_controller.h
// Date  ：2026/09/09
// Author：Miu Kitamura
// 
// ・シーン共有AIの所有・初期化・更新を管理するBehavior。
// ・敵個体の状態や行動選択は、個体側のBehaviorで管理する。
//---------------------------------------------------
#ifndef ENEMY_AI_CONTROLLER_H
#define ENEMY_AI_CONTROLLER_H
#include "Engine/Component/behavior_component.h"

#include "enemy_ai_world_context.h"
#include "meta_ai.h"
#include "navigation_system.h"
#include "tactical_query_system.h"

class EnemyAIController : public BehaviorComponent {
private:
    EnemyAIWorldContext m_context;

    MetaAI m_metaAI;
    NavigationSystem m_navigation;
    TacticalQuerySystem m_tacticalQuery;

    bool m_isInitialized = false;

public:
    EnemyAIController() = default;
    ~EnemyAIController() override;

    void Start() override;
    void Update() override;
    void DrawComponentInspector() override;

    bool IsInitialized() const { return m_isInitialized; }

    // === AIシステムの参照取得 ===
    MetaAI& GetMetaAI() { return m_metaAI; }
    const MetaAI& GetMetaAI() const { return m_metaAI; }
    NavigationSystem& GetNavigationSystem() { return m_navigation; }
    const NavigationSystem& GetNavigationSystem() const { return m_navigation; }
    TacticalQuerySystem& GetTacticalQuerySystem() { return m_tacticalQuery; }
    const TacticalQuerySystem& GetTacticalQuerySystem() const { return m_tacticalQuery; }

};

#endif // ENEMY_AI_CONTROLLER_H
