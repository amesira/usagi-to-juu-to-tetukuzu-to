// enemy_ai_world_context.h
// 2026/09/09
#pragma once
#include <limits>
#include <vector>
#include "enemy_ai_world_settings_asset.h"

namespace EnemyAiWorld {
    // === グリッド情報 ===
    struct GridCoord {
        int x = 0;
        int z = 0;
    };

    enum class CellType {
        Unknown,
        NoGround,   // 地面が無い
        Ground,     // 通常の地面
        Obstacle,   // 通行不可の障害物
        SteepSlope, // 斜面
    };

    struct GridCell {
        CellType type = CellType::Unknown;
        float height = 0.0f;
        DirectX::XMFLOAT3 normal = { 0.0f, 1.0f, 0.0f };
    };

    // === Entity情報 ===
    // MetaAIが更新する参照用スナップショット。Component本体の値は変更しない
    // IDはシーン内でのみ有効。破棄・ID再利用時に登録情報を更新すること
    struct EntityInfo {
        unsigned int gameObjectID = -1;
        DirectX::XMFLOAT3 position = {};
        DirectX::XMFLOAT3 velocity = {};
        float radius = 0.0f;
    };

    // === Navigation情報 ===
    enum class PathQueryStatus {
        NotReady,
        Success,
        Unreachable,
        InvalidStart,
        InvalidGoal,
    };

    struct NavigationPath {
        // World座標。追従中のインデックスは敵個体側で保持する。
        std::vector<DirectX::XMFLOAT3> waypoints;
    };

    struct PathQueryResult {
        PathQueryStatus status = PathQueryStatus::NotReady;
        NavigationPath path;
    };

    // === Tactical情報 ===
    //struct TacticalCellInfo {
    //    
    //};

    //struct TacticalQueryWeights {
    //    float targetDistance = 1.0f;
    //    float enemyDensity = 1.0f;
    //    float danger = 1.0f;
    //    float lineOfSight = 1.0f;
    //    float pathCost = 1.0f;
    //};

    //struct TacticalQueryRequest {
    //    unsigned int requesterID = -1;
    //    DirectX::XMFLOAT3 startPosition = {}; // 経路コストの起点
    //    DirectX::XMFLOAT3 searchCenter = {};
    //    float searchRadius = 0.0f;
    //    EntityInfo target;
    //    float desiredTargetDistance = 0.0f;
    //    NavigationAgentSettings agentSettings;
    //    TacticalQueryWeights weights;
    //};

    struct TacticalQueryResult {
        bool found = false;
        DirectX::XMFLOAT3 position = {}; // found == trueのときのみ有効
        float score = 0.0f; // 大きいほど高評価。found == trueのときのみ有効
    };
}

struct EnemyAIWorldRuntimeState {

};

struct EnemyAIWorldContext {
    class IScene* scene = nullptr;
    class EnemyAIController* controller = nullptr;

    class MetaAI* metaAI = nullptr;
    class NavigationSystem* navigation = nullptr;
    class TacticalQuerySystem* tacticalQuery = nullptr;

    EnemyAIWorldRuntimeState runtimeState;
    const EnemyAiWorldSettingsAsset* settingsAsset = nullptr;
    const EnemyAiWorldSettings::Data& settings() const {
        static const EnemyAiWorldSettings::Data defaults;
        return settingsAsset ? settingsAsset->GetData() : defaults;
    }

};
