#pragma once
#include "Engine/Component/behavior_component.h"
#include "wave_progress.h"
#include <DirectXMath.h>
#include <string>
#include <vector>
#include <array>
#include <random>
#include <deque>
#include <cstdint>

struct WaveDefeatEvent {
    std::uint64_t serial;
    int points;
    DirectX::XMFLOAT3 position;
};

enum class WaveEnemyType { GroundMelee, HoverRanged, EliteGroundMelee, EliteHoverRanged };

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
    std::array<std::string, 4> m_definitionPaths = {{
        "asset/Data/enemy_ground_melee.definition.data.json",
        "asset/Data/enemy_hover_ranged.definition.data.json",
        "asset/Data/enemy_elite_ground_melee.definition.data.json",
        "asset/Data/enemy_elite_hover_ranged.definition.data.json"
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
    std::deque<WaveDefeatEvent> m_defeatEvents;
    std::uint64_t m_defeatSerial = 0;

public:
    ~WaveControllerBehavior() override;
    void Start() override;
    void Update() override;
    void DrawComponentInspector() override;
    // 敵の被弾処理から通知。生成済み敵に一度だけ加点し、ここでは破棄しない。
    void NotifyEnemyDefeated(unsigned int id);
    const WaveProgress& GetProgress() const { return m_progress; }
    int GetWaveCount() const { return m_settings.waveCount; }
    int GetAliveEnemyCount() const;
    const std::deque<WaveDefeatEvent>& GetDefeatEvents() const { return m_defeatEvents; }
    std::uint64_t GetDefeatSerial() const { return m_defeatSerial; }

private:
    void CollectEnemies(IScene* scene, EnemyAIWorldController* aiWorld, float deltaTime);
    void StopAllEnemies(IScene* scene, EnemyAIWorldController* aiWorld);
    void CleanupRemainingEnemies(IScene* scene, EnemyAIWorldController* aiWorld);
    bool TrySpawnEnemy(IScene* scene, EnemyAIWorldController& aiWorld);
    int CountSceneEnemies(IScene* scene) const;
};
