#pragma once
#include "Engine/Component/behavior_component.h"
#include "wave_progress.h"
#include <DirectXMath.h>
#include <string>
#include <vector>
#include <array>
#include <random>

enum class WaveEnemyType { GroundMelee, HoverRanged, EliteGroundMelee, EliteHoverRanged };

struct WaveEnemyDefinition {
    WaveEnemyType type;
    int defeatPoints;
    float maxHealth;
    float scale;
    const char* agentPath;
    const char* movePath;
    const char* approachPath = "asset/Data/enemy_approach_settings.data.json";
    const char* attackPath = "asset/Data/enemy_attack_settings.data.json";
};
struct WaveEnemyEntry { int weight; int maxAlive; };
struct WaveComposition {
    std::array<WaveEnemyEntry, 4> enemies;
    WaveComposition(WaveEnemyEntry ground, WaveEnemyEntry hover, WaveEnemyEntry eliteGround, WaveEnemyEntry eliteHover)
        : enemies{{ground, hover, eliteGround, eliteHover}} {}
};
struct WaveSpawnPoint {
    DirectX::XMFLOAT3 position;
    bool allowGround = true;
    bool allowHover = true;
};

class IScene;
class EnemyAIWorldController;

class WaveControllerBehavior : public BehaviorComponent {
    struct SpawnedEnemy {
        unsigned int id;
        std::string name; // IDスロットの再利用を識別する生成時トークン
        WaveEnemyType type;
        int defeatPoints;
        bool credited = false;
        float deadTime = 0.0f;
    };
    WaveSettings m_settings;
    WaveProgress m_progress;
    std::vector<SpawnedEnemy> m_enemies;
    std::array<WaveEnemyDefinition, 4> m_definitions = {{
        {WaveEnemyType::GroundMelee, 10, 100, 1, "asset/Data/enemy_ai_agent_settings.data.json", "asset/Data/enemy_move_settings.data.json"},
        {WaveEnemyType::HoverRanged, 15, 100, 1, "asset/Data/enemy_hover_ai_agent_settings.data.json", "asset/Data/enemy_hover_move_settings.data.json"},
        {WaveEnemyType::EliteGroundMelee, 25, 200, 1.2f, "asset/Data/enemy_elite_ai_agent_settings.data.json", "asset/Data/enemy_move_settings.data.json"},
        {WaveEnemyType::EliteHoverRanged, 30, 200, 1.2f, "asset/Data/enemy_elite_hover_ai_agent_settings.data.json", "asset/Data/enemy_hover_move_settings.data.json"}
    }};
    // 5ウェーブ目以降は最後の構成を再利用する。
    std::array<WaveComposition, 5> m_compositions = {{
        {{100, 6}, {0, 0}, {0, 0}, {0, 0}},
        {{70, 6}, {30, 2}, {0, 0}, {0, 0}},
        {{60, 6}, {25, 2}, {15, 1}, {0, 0}},
        {{45, 6}, {30, 3}, {15, 1}, {10, 1}},
        {{35, 6}, {35, 3}, {15, 1}, {15, 1}}
    }};
    std::mt19937 m_random{std::random_device{}()};
    std::vector<WaveSpawnPoint> m_spawnPoints = {
        {{-15, 0, -15}}, {{15, 0, -15}}, {{-15, 0, 15}},
        {{15, 0, 15}}, {{-25, 0, 0}}, {{25, 0, 0}}
    };
    float m_spawnTimer = 0.0f;
    unsigned int m_spawnSerial = 0;
    size_t m_nextSpawnPoint = 0;
    std::string m_status = "Waiting for AI world / player";

public:
    ~WaveControllerBehavior() override;
    void Start() override;
    void Update() override;
    void DrawComponentInspector() override;
    // 敵の被弾処理から通知。生成済み敵に一度だけ加点し、ここでは破棄しない。
    void NotifyEnemyDefeated(unsigned int id);
    const WaveProgress& GetProgress() const { return m_progress; }

private:
    void CollectEnemies(IScene* scene, EnemyAIWorldController* aiWorld, float deltaTime);
    bool TrySpawnEnemy(IScene* scene, EnemyAIWorldController& aiWorld);
    int CountSceneEnemies(IScene* scene) const;
};
