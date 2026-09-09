// enemy_ai_context.h
// 2026/09/09
#pragma once

namespace EnemyAI {

    struct GridCell {

    };
}

struct EnemyAIRuntimeState {

};

struct EnemyAIContext {
    class EnemyAIController* controller = nullptr;

    class MetaAI* metaAI = nullptr;
    class NavigationSystem* navigation = nullptr;
    class TacticalQuerySystem* tacticalQuery = nullptr;

    EnemyAIRuntimeState runtimeState;
};