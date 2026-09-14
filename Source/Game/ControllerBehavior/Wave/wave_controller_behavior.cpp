#include "wave_controller_behavior.h"
#include "Game/ControllerBehavior/game_controller_locator.h"
#include "Game/ControllerBehavior/EnemyAI/enemy_ai_world_controller.h"
#include "Game/ActorBehavior/Enemy/enemy_behavior.h"
#include "Game/ActorBehavior/Player/player_behavior.h"
#include "Game/ActorBehavior/Base/health_behavior.h"
#include "Game/Factory/prefab_factory.h"
#include "Game/Factory/Prefab/enemy_definition_asset.h"
#include <cstdio>
#include "Engine/Core/game_object.h"
#include "Engine/Core/scene_interface.h"
#include "Engine/Component/transform_component.h"
#include "Engine/Component/collider_component.h"
#include "Engine/Device/mi_fps.h"
#include "Engine/Editor/LevelEditor/behavior_detail_view.h"
#include "Engine/engine_service_locator.h"
#include <algorithm>
#include <cmath>

#include "Game/ControllerBehavior/Audio/game_audio_controller_behavior.h"

WaveControllerBehavior::~WaveControllerBehavior()
{
    if (GameControllerLocator::s_waveController == this)
        GameControllerLocator::s_waveController = nullptr;
}

void WaveControllerBehavior::Start()
{
    if (!GetOwner() || !GetOwner()->GetScene()) return;
    auto* current = GameControllerLocator::s_waveController;
    if (current && current != this && current->GetEnable()) {
        SetEnable(false);
        return;
    }
    GameControllerLocator::s_waveController = this;
    // エディタのAsset型登録もGetAssetで行われる。欠落時に別種の初期値を保存しない。
    for (const auto& path : m_definitionPaths) DATA_LOADER->GetAsset<EnemyDefinitionAsset>(path);

    // 最初のBGMを再生
    if (auto* audio = GameControllerLocator::Audio()) {
        audio->SetBgm(GameBgm::Battle);
    }
}

void WaveControllerBehavior::NotifyEnemyDefeated(unsigned int id)
{
    if (Game::Wave() != this || !GetEnable() || !GetOwner()) return;

    auto* scene = GetOwner()->GetScene();
    if (!scene) return;

    auto* object = scene->GetGameObjectByID(id);
    for (auto& enemy : m_enemies) {
        if (enemy.id != id || enemy.credited || !object || object->GetName() != enemy.name) continue;
        auto* health = object->GetComponent<HealthBehavior>();
        if (!health || !health->IsDead()) return;
        enemy.credited = true;
        const int previousScore = m_progress.totalScore;
        auto* behavior = object->GetComponent<EnemyBehavior>();
        if (behavior && behavior->GetDeathReason() == EnemyDeathReason::WaveCleanup) return;
        m_progress.AddDefeatPoints(enemy.defeatPoints, m_settings);
        const int gained = m_progress.totalScore - previousScore;
        if (gained > 0) {
            if (auto* transform = object->GetComponent<TransformComponent>()) {
                m_defeatEvents.push_back({++m_defeatSerial, gained, transform->GetPosition()});
                if (m_defeatEvents.size() > 64) m_defeatEvents.pop_front();
            }
        }
        return;
    }
}

void WaveControllerBehavior::CollectEnemies(IScene* scene, EnemyAIWorldController* aiWorld, float deltaTime)
{
    std::erase_if(m_enemies, [&](SpawnedEnemy& enemy) {
        auto* object = scene->GetGameObjectByID(enemy.id);
        if (object && object->GetName() != enemy.name) return true;
        if (!object || !object->GetActive()) {
            if (aiWorld) {
                aiWorld->CancelAttackRequest(static_cast<int>(enemy.id));
                aiWorld->GetMetaAI().UnregisterEnemy(enemy.id);
            }
            return true; // 外部削除は加点しない
        }
        auto* health = object->GetComponent<HealthBehavior>();
        if (!health || !health->IsDead()) return false;
        enemy.deadTime += deltaTime;
        if (enemy.deadTime < m_settings.corpseDuration) return false;
        if (aiWorld) {
            aiWorld->CancelAttackRequest(static_cast<int>(enemy.id));
            aiWorld->GetMetaAI().UnregisterEnemy(enemy.id);
        }
        health->SetUiActive(false);
        object->SetActive(false);
        object->Destroy();
        return true;
    });
}

void WaveControllerBehavior::StopAllEnemies(IScene* scene, EnemyAIWorldController* aiWorld)
{
    for (auto& enemy : m_enemies) {
        auto* object = scene->GetGameObjectByID(enemy.id);
        if (!object || !object->GetActive() || object->GetName() != enemy.name) continue;
        if (auto* behavior = object->GetComponent<EnemyBehavior>()) {
            behavior->StartStun(5.0f);
        }
    }
}

void WaveControllerBehavior::CleanupRemainingEnemies(IScene* scene, EnemyAIWorldController* aiWorld)
{
    // Close the scoring window before invoking any enemy death processing.
    m_progress.MarkCleanupIssued();
    for (auto& enemy : m_enemies) {
        auto* object = scene->GetGameObjectByID(enemy.id);
        if (!object || !object->GetActive() || object->GetName() != enemy.name) continue;
        auto* health = object->GetComponent<HealthBehavior>();
        if (!health || health->IsDead()) continue;
        enemy.credited = true;
        if (aiWorld) {
            aiWorld->CancelAttackRequest(static_cast<int>(enemy.id));
            aiWorld->GetMetaAI().UnregisterEnemy(enemy.id);
        }
        if (auto* behavior = object->GetComponent<EnemyBehavior>()) behavior->RequestWaveCleanup();
        else { health->SetHealth(0); health->SetUiActive(false); }
    }
}

int WaveControllerBehavior::GetAliveEnemyCount() const
{
    auto* scene = GetOwner() ? GetOwner()->GetScene() : nullptr;
    if (!scene) return 0;
    int count = 0;
    for (const auto& enemy : m_enemies) {
        auto* object = scene->GetGameObjectByID(enemy.id);
        if (!object || !object->GetActive() || object->GetName() != enemy.name) continue;
        auto* health = object->GetComponent<HealthBehavior>();
        if (health && !health->IsDead()) ++count;
    }
    return count;
}

int WaveControllerBehavior::CountSceneEnemies(IScene* scene) const
{
    int count = 0;
    for (auto& object : scene->GetGameObjects())
        if (object.GetActive() && object.GetComponent<EnemyBehavior>()) ++count;
    return count;
}

bool WaveControllerBehavior::TrySpawnEnemy(IScene* scene, EnemyAIWorldController& aiWorld)
{
    if (m_spawnPoints.empty()) return false;
    const auto& composition = m_compositions[std::clamp(m_progress.waveNumber - 1, 0, 4)];
    std::array<int, 4> counts{};
    for (const auto& enemy : m_enemies) ++counts[static_cast<size_t>(enemy.type)];
    std::array<int, 4> weights{};
    std::array<EnemyDefinitionAsset*, 4> definitions{};
    int totalWeight = 0;
    for (size_t i = 0; i < weights.size(); ++i) {
        definitions[i] = DATA_LOADER->GetAsset<EnemyDefinitionAsset>(m_definitionPaths[i]);
        if (!definitions[i]) continue;
        const bool hover = definitions[i]->GetData().hover;
        const bool allowed = std::any_of(m_spawnPoints.begin(), m_spawnPoints.end(),
            [hover](const auto& point) { return hover ? point.allowHover : point.allowGround; });
        if (allowed && counts[i] < composition.enemies[i].maxAlive)
            weights[i] = (std::max)(composition.enemies[i].weight, 0);
        totalWeight += weights[i];
    }
    if (totalWeight == 0) { m_status = "No eligible enemy type"; return false; }
    int choice = std::uniform_int_distribution<int>(1, totalWeight)(m_random);
    size_t typeIndex = 0;
    for (; typeIndex < weights.size() - 1; ++typeIndex) {
        choice -= weights[typeIndex];
        if (choice <= 0) break;
    }
    const auto definition = definitions[typeIndex]->GetData();
    const bool hover = definition.hover;
    auto* asset = DATA_LOADER->GetAsset<EnemyAiAgentSettingsAsset>(
        definition.agentPath);
    auto* moveAsset = DATA_LOADER->GetAsset<EnemyMoveSettingsAsset>(definition.movePath);
    if (!asset || !moveAsset || moveAsset->GetData().hoverEnabled != hover) {
        m_status = "Enemy definition: missing AI / Move settings or Hover mismatch";
        return false;
    }
    const auto agent = EnemyDefinition::ResolveAgent(definition, asset->GetData()).navigationAgent;
    auto& navigation = aiWorld.GetNavigationSystem();
    const auto player = aiWorld.GetMetaAI().GetPlayerPosition();
    for (size_t attempt = 0; attempt < m_spawnPoints.size(); ++attempt) {
        const size_t index = m_nextSpawnPoint++ % m_spawnPoints.size();
        const auto& point = m_spawnPoints[index];
        if (hover ? !point.allowHover : !point.allowGround) continue;
        auto position = point.position;
        EnemyAiWorld::GridCoord coord;
        if (!navigation.WorldToGrid(position, coord) || !navigation.IsWalkable(coord, agent)) continue;
        const auto* cell = navigation.GetCell(coord);
        if (!cell) continue;
        position.y = cell->height + 0.1f;
        const float dx = position.x - player.x;
        const float dz = position.z - player.z;
        if (dx * dx + dz * dz < 25.0f) continue;
        bool occupied = false;
        for (auto& object : scene->GetGameObjects()) {
            if (!object.GetActive() || !object.GetComponent<EnemyBehavior>()) continue;
            auto* transform = object.GetComponent<TransformComponent>();
            if (!transform) continue;
            const auto other = transform->GetPosition();
            const float x = other.x - position.x;
            const float z = other.z - position.z;
            if (x * x + z * z < 9.0f) { occupied = true; break; }
        }
        if (occupied) continue;
        if (aiWorld.FindPath(position, player, agent).status != EnemyAiWorld::PathQueryStatus::Success) continue;
        if (hover) position.y += moveAsset ? moveAsset->GetData().hoverHeight : 3.0f;

        auto prefab = PrefabFactory::CreateEnemyFromDefinition(scene, position, definition);

        if (!prefab.enemy) {
            m_status = "Enemy definition: creation failed; check settings references";
            return false;
        }
        const std::string name = "WaveEnemy_" + std::to_string(++m_spawnSerial);
        prefab.enemy->SetName(name);
        m_enemies.push_back({prefab.enemy->GetID(), name, static_cast<WaveEnemyType>(typeIndex), definition.defeatPoints});
        m_status = "Enemy type " + std::to_string(typeIndex) + " spawned";
        return true;
    }
    m_status = "No valid spawn point: check navigation, distance and occupancy";
    return false;
}

void WaveControllerBehavior::Update()
{
    if (Game::Wave() != this || !GetOwner()) return;

    auto* scene = GetOwner()->GetScene();
    if (!scene) return;

    const float deltaTime = FPS_GetDeltaTime();
    if (!std::isfinite(deltaTime) || deltaTime < 0.0f) return;

    auto* aiWorld = Game::EnemyAIWorld();
    CollectEnemies(scene, aiWorld, deltaTime);

    bool playerDead = false;
    bool hasPlayer = false;
    for (auto& object : scene->GetGameObjects()) {
        if (!object.GetActive() || !object.GetComponent<PlayerBehavior>()) continue;
        hasPlayer = true;
        if (auto* health = object.GetComponent<HealthBehavior>()) playerDead = health->IsDead();
        break;
    }
    if (playerDead) m_progress.state = WaveProgress::State::GameOver;

    const bool ready = hasPlayer && aiWorld && aiWorld->GetEnable()
        && aiWorld->IsInitialized() && aiWorld->GetMetaAI().HasPlayer();

    const auto previousState = m_progress.state;
    m_progress.Update(deltaTime, ready, static_cast<int>(m_enemies.size()), m_settings);

    if (m_progress.ShouldCleanupEnemies()) {
        CleanupRemainingEnemies(scene, aiWorld);
    }
    if (m_progress.state != WaveProgress::State::Battle || !ready) return;
    if (previousState != WaveProgress::State::Battle) m_spawnTimer = 0.0f;

    m_spawnTimer = (std::max)(m_spawnTimer - deltaTime, 0.0f);
    if (m_spawnTimer > 0.0f) return;

    // 同期生成は1フレーム1体まで。死体も含めたシーン全体の敵数を制限する。
    if (CountSceneEnemies(scene) >= std::clamp(m_settings.maxConcurrentEnemies, 1, 10)) return;
    TrySpawnEnemy(scene, *aiWorld);
    m_spawnTimer = (std::max)(m_settings.spawnInterval, 0.1f);
}

void WaveControllerBehavior::DrawComponentInspector()
{
    if (BehaviorDetailView::BeginSection(this, "Wave Controller")) {
        const char* states[] = {"Waiting", "Preparing", "Battle", "Intermission", "Complete", "Game Over"};
        ImGui::Text("State: %s", states[static_cast<int>(m_progress.state)]);
        ImGui::Text("Wave: %d / %d", m_progress.waveNumber, m_settings.waveCount);
        ImGui::Text("Points: %d / %d | Total: %d", m_progress.wavePoints, m_progress.targetPoints, m_progress.totalScore);
        ImGui::Text("Tracked enemies (including corpses): %d", static_cast<int>(m_enemies.size()));
        if (GetOwner() && GetOwner()->GetScene()) ImGui::Text("Scene enemies: %d", CountSceneEnemies(GetOwner()->GetScene()));
        ImGui::Text("Phase timer: %.2f | Spawn timer: %.2f", m_progress.remainingTime, m_spawnTimer);
        ImGui::TextWrapped("%s", m_status.c_str());
        // 進行中の目標変更は避け、設定変更は次ウェーブ開始時に反映する。
        ImGui::SliderInt("Wave Count", &m_settings.waveCount, 1, 100);
        ImGui::SliderInt("First Target", &m_settings.firstTargetPoints, 1, 10000);
        ImGui::SliderInt("Target Increment", &m_settings.targetPointsIncrement, 0, 1000);
        const char* types[] = {"Ground Melee", "Hover Ranged", "Elite Ground Melee", "Elite Hover Ranged"};
        if (ImGui::TreeNode("Enemy Definitions")) {
            for (size_t i = 0; i < m_definitionPaths.size(); ++i) {
                ImGui::PushID(static_cast<int>(i));
                ImGui::TextUnformatted(types[i]);
                char path[512];
                std::snprintf(path, sizeof(path), "%s", m_definitionPaths[i].c_str());
                if (ImGui::InputText("Definition Asset", path, sizeof(path))) m_definitionPaths[i] = path;
                if (auto* asset = DATA_LOADER->GetAsset<EnemyDefinitionAsset>(m_definitionPaths[i])) {
                    const auto& definition = asset->GetData();
                    ImGui::Text("HP %.0f | Points %d | Scale %.2f", definition.maxHealth, definition.defeatPoints, definition.scale);
                }
                else ImGui::TextUnformatted("Definition asset not found");
                ImGui::PopID();
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Wave Compositions")) {
            for (size_t wave = 0; wave < m_compositions.size(); ++wave) {
                ImGui::PushID(static_cast<int>(wave));
                ImGui::Text("Wave %d", static_cast<int>(wave + 1));
                for (size_t i = 0; i < 4; ++i) {
                    ImGui::PushID(static_cast<int>(i));
                    ImGui::TextUnformatted(types[i]);
                    ImGui::SliderInt("Weight", &m_compositions[wave].enemies[i].weight, 0, 100);
                    ImGui::SliderInt("Type Limit", &m_compositions[wave].enemies[i].maxAlive, 0, 10);
                    ImGui::PopID();
                }
                ImGui::PopID();
            }
            ImGui::TreePop();
        }
        ImGui::SliderInt("Concurrent Limit", &m_settings.maxConcurrentEnemies, 1, 10);
        ImGui::SliderFloat("Spawn Interval", &m_settings.spawnInterval, 0.1f, 10.0f);
        ImGui::SliderFloat("Preparation", &m_settings.preparationDuration, 0.0f, 10.0f);
        ImGui::SliderFloat("Intermission", &m_settings.intermissionDuration, 0.0f, 10.0f);
        ImGui::SliderFloat("Cleanup Delay", &m_settings.cleanupDelay, 0.0f, 10.0f);
        if (ImGui::TreeNode("Spawn Points (XZ; Y resolved from navigation)")) {
            for (size_t i = 0; i < m_spawnPoints.size(); ++i) {
                ImGui::PushID(static_cast<int>(i));
                ImGui::DragFloat3("Position", &m_spawnPoints[i].position.x, 0.1f);
                ImGui::Checkbox("Ground", &m_spawnPoints[i].allowGround);
                ImGui::Checkbox("Hover", &m_spawnPoints[i].allowHover);
                ImGui::PopID();
            }
            ImGui::TreePop();
        }
    }
    BehaviorDetailView::EndSection();
}
