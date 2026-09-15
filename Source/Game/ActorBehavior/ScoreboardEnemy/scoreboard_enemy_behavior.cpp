#include "scoreboard_enemy_behavior.h"
#include "Game/ControllerBehavior/ScoreboardEnemy/scoreboard_enemy_settings_asset.h"
#include "Game/ControllerBehavior/game_controller_locator.h"
#include "Game/ControllerBehavior/EnemyAI/enemy_ai_world_controller.h"
#include "Game/ControllerBehavior/EnemyAI/enemy_ai_agent_settings_asset.h"
#include "Engine/Core/game_object.h"
#include "Engine/Core/scene_interface.h"
#include "Engine/Component/transform_component.h"
#include "Engine/Component/model_component.h"
#include "Engine/Component/animation_component.h"
#include "Engine/Component/image_component.h"
#include "Engine/Component/text_component.h"
#include "Engine/Device/mi_fps.h"
#include "Engine/engine_service_locator.h"
#include "Utility/mi_math.h"
#include <algorithm>
#include <cmath>

void ScoreboardEnemyBehavior::Setup(const ScoreboardEnemySettingsAsset* settings,
    const ScoreRecord& record, std::string label) {
    m_settings = settings; m_record = record; m_label = std::move(label);
}

char ScoreboardEnemyBehavior::RankLetter(int rank) {
    static constexpr char letters[] = {'D', 'C', 'B', 'A', 'S'};
    return letters[std::clamp(rank, 1, 5) - 1];
}

DirectX::XMFLOAT4 ScoreboardEnemyBehavior::RankColor() const {
    const auto& s = m_settings->GetData();
    switch (std::clamp(m_record.rank, 1, 5)) {
    case 5: return s.rankSColor; case 4: return s.rankAColor; case 3: return s.rankBColor;
    case 2: return s.rankCColor; default: return s.rankDColor;
    }
}

void ScoreboardEnemyBehavior::Start() {
    if (!m_settings || !GetOwner() || !GetOwner()->GetScene()) return;
    m_transform = GetOwner()->GetComponent<TransformComponent>();
    if (!m_transform) return;
    m_pathFollower.Initialize(m_settings->GetData().waypointReachDistance);
    m_animationContext.scene = GetOwner()->GetScene();
    m_animationContext.transform = m_transform;
    m_animation.Initialize(m_animationContext);
    if (auto* model = GetOwner()->GetComponent<ModelComponent>()) {
        const auto color = RankColor();
        for (auto& slot : model->GetMaterialSlots()) {
            slot.isOverrideBaseColor = slot.isOverrideEmissive = true;
            slot.overrideBaseColor = color;
            slot.overrideEmissiveColor = {color.x, color.y, color.z};
            slot.overrideEmissiveIntensity = m_settings->GetData().emissiveIntensity;
        }
    }
    CreateBoard();
}

void ScoreboardEnemyBehavior::CreateBoard() {
    auto* scene = GetOwner()->GetScene();
    const auto& s = m_settings->GetData();
    auto* panel = scene->CreateGameObject(); panel->SetName("ScoreboardEnemy.Panel");
    auto* pt = panel->AddComponent<TransformComponent>(); pt->SetScaling(s.panelScale);
    auto* image = panel->AddComponent<ImageComponent>();
    image->SetTextureResource(TEXTURE_REPOSITORY->GetTextureResource(L"asset/Texture/white.bmp"));
    image->SetColor(s.panelColor); image->SetWorldSpaceType(ImageComponent::WorldSpaceType::Billboard);
    m_uiIds[0] = panel->GetID();
    auto makeText = [&](int index, const std::string& value, float y, int size, DirectX::XMFLOAT4 color) {
        auto* object = scene->CreateGameObject(); object->SetName("ScoreboardEnemy.Text");
        object->AddComponent<TransformComponent>()->SetScaling({1,1,1});
        auto* text = object->AddComponent<TextComponent>(); text->SetText(value); text->SetCenter(true);
        text->SetFontSize(size); text->SetFontPath("asset/Font/Makinas-4-Square.otf"); text->SetColor(color);
        m_uiIds[index] = object->GetID();
    };
    makeText(1, m_label, 0.25f, 32, s.textColor);
    makeText(2, std::to_string(m_record.totalScore) + "  RANK " + RankLetter(m_record.rank), -0.3f, 28, RankColor());
    UpdateBoard();
}

void ScoreboardEnemyBehavior::UpdateBoard() {
    if (!m_transform || !GetOwner() || !m_settings) return;
    auto* scene = GetOwner()->GetScene();
    const auto base = MiMath::Add(m_transform->GetPosition(), m_settings->GetData().boardOffset);
    const float ys[] = {0.0f, 0.28f, -0.3f};
    for (size_t i = 0; i < m_uiIds.size(); ++i) {
        auto* object = scene->GetGameObjectByID(m_uiIds[i]);
        auto* transform = object ? object->GetComponent<TransformComponent>() : nullptr;
        if (transform) transform->SetPosition({base.x, base.y + ys[i], base.z});
    }
}

void ScoreboardEnemyBehavior::Update() {
    if (!m_transform || !m_settings || !GetOwner()) return;
    auto* ai = Game::EnemyAIWorld();
    if (!ai || !ai->IsInitialized() || !ai->GetMetaAI().HasPlayer()) return;
    if (!m_registered) { ai->GetMetaAI().RegisterEnemy(GetOwner()); m_registered = true; }
    const float dt = FPS_GetDeltaTime();
    if (!std::isfinite(dt) || dt <= 0) return;
    const auto current = m_transform->GetPosition();
    const auto target = ai->GetMetaAI().GetPlayerPosition();
    const float dx = target.x-current.x, dz = target.z-current.z;
    const float distance = std::hypot(dx, dz);
    DirectX::XMFLOAT3 velocity{};
    if (distance > m_settings->GetData().stopDistance) {
        m_repathTimer -= dt;
        if (m_repathTimer <= 0 || !m_pathFollower.HasPath() || m_pathFollower.HasReachedGoal()) {
            auto* asset = DATA_LOADER->GetAsset<EnemyAiAgentSettingsAsset>(m_settings->GetData().agentSettingsPath, true);
            if (asset) {
                auto result = ai->FindPath(current, target, asset->GetData().navigationAgent,
                    static_cast<int>(GetOwner()->GetID()));
                if (result.status == EnemyAiWorld::PathQueryStatus::Success) m_pathFollower.SetPath(std::move(result.path));
            }
            m_repathTimer = (std::max)(0.05f, m_settings->GetData().repathInterval);
        }

        // Transformを更新する純粋な移動
        m_pathFollower.Update(current);
        const auto direction = m_pathFollower.GetMoveDirection();
        velocity = MiMath::Multiply(direction, (std::max)(0.0f, m_settings->GetData().moveSpeed));
        m_transform->SetPosition(MiMath::Add(current, MiMath::Multiply(velocity, dt)));
        if (std::hypot(direction.x, direction.z) > 0.001f)
            m_transform->SetRotation(MiMath::LookRotation(direction, {0,1,0}));
    }
    m_animationContext.runtimeState.controlVelocity = velocity;
    m_animation.Update(m_animationContext);
    UpdateBoard();
}

void ScoreboardEnemyBehavior::OnDestroy() {
    if (m_registered) if (auto* ai = Game::EnemyAIWorld()) ai->GetMetaAI().UnregisterEnemy(GetOwner()->GetID());
    m_registered = false; m_animation.Finalize();
    if (GetOwner() && GetOwner()->GetScene()) for (auto id : m_uiIds) {
        if (id == InvalidObjectId) continue;
        if (auto* object = GetOwner()->GetScene()->GetGameObjectByID(id)) object->Destroy();
    }
}

void ScoreboardEnemyBehavior::DrawComponentInspector() {}
