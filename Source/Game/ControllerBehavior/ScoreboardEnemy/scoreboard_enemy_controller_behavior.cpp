#include "scoreboard_enemy_controller_behavior.h"
#include "scoreboard_enemy_settings_asset.h"
#include "Game/ActorBehavior/ScoreboardEnemy/scoreboard_enemy_behavior.h"
#include "Game/ControllerBehavior/Result/score_save_store.h"
#include "Game/ControllerBehavior/game_controller_locator.h"
#include "Game/ControllerBehavior/EnemyAI/enemy_ai_world_controller.h"
#include "Engine/Core/game_object.h"
#include "Engine/Core/scene_interface.h"
#include "Engine/Component/transform_component.h"
#include "Engine/Component/model_component.h"
#include "Engine/Component/animation_component.h"
#include "Engine/Component/collider_component.h"
#include "Engine/Component/rigidbody_component.h"
#include "Engine/Graphics/model_repository.h"
#include "Engine/engine_service_locator.h"
#include "Engine/Editor/LevelEditor/behavior_detail_view.h"

void ScoreboardEnemyControllerBehavior::Update() {
    if (m_created || !m_settings || !Game::EnemyAIWorld() || !Game::EnemyAIWorld()->IsInitialized()) return;
    CreateEnemies();
}

void ScoreboardEnemyControllerBehavior::CreateEnemies() {
    auto* scene = GetOwner() ? GetOwner()->GetScene() : nullptr;
    if (!scene) return;
    m_created = true;
    const auto& save = ScoreSaveStore::Get();
    const auto& settings = m_settings->GetData();
    auto create = [&](const char* name, const DirectX::XMFLOAT3& position,
        const ScoreRecord& record, const char* label) {
        auto* object = scene->CreateGameObject(); object->SetName(name);
        object->SetTag("ScoreboardEnemy"); object->SetCollisionLayer(CollisionLayer::Enemy);
        object->SetRenderLayer(RenderLayer::Enemy);
        auto* transform = object->AddComponent<TransformComponent>();
        transform->SetPosition(position); transform->SetScaling({settings.modelScale, settings.modelScale, settings.modelScale});
        auto* collider = object->AddComponent<CapsuleColliderComponent>();
        collider->SetRadius(settings.modelScale); collider->SetHeight(settings.modelScale);
        collider->SetCenter({0, 0.8f * settings.modelScale, 0});
        auto* body = object->AddComponent<RigidbodyComponent>();
        body->SetGravityScale(0);
        body->SetIsKinematic(false);
        auto* model = object->AddComponent<ModelComponent>();
        if (auto* resource = MODEL_REPOSITORY->GetModel(settings.modelPath)) model->SetModelResource(resource);
        object->AddComponent<AnimationComponent>();
        object->AddComponent<ScoreboardEnemyBehavior>()->Setup(m_settings, record, label);
    };
    if (save.hasHighScore) create("HighScoreEnemy", settings.highScoreSpawn, save.highScore, "HIGH SCORE");
    if (save.hasPreviousScore) create("PreviousScoreEnemy", settings.previousScoreSpawn, save.previousScore, "LAST SCORE");
}

void ScoreboardEnemyControllerBehavior::DrawComponentInspector() {
    if (BehaviorDetailView::BeginSection(this, "Scoreboard Enemy Controller")) {
        ImGui::Text("Created: %s", m_created ? "Yes" : "No");
        ImGui::TextUnformatted("Settings: asset/Data/scoreboard_enemy_settings.data.json");
    }
    BehaviorDetailView::EndSection();
}
