#include "stage_decoration_controller_behavior.h"
#include "Game/ControllerBehavior/game_controller_locator.h"
#include "Engine/Core/game_object.h"
#include "Engine/Core/scene_interface.h"
#include "Engine/Component/transform_component.h"
#include "Engine/Component/model_component.h"
#include "Engine/Graphics/shader_definitions.h"
#include "Engine/Device/mi_fps.h"
#include "Engine/engine_service_locator.h"
#include <filesystem>

StageDecorationControllerBehavior::~StageDecorationControllerBehavior()
{
    if (GameControllerLocator::s_stageDecorationController == this)
        GameControllerLocator::s_stageDecorationController = nullptr;
}

void StageDecorationControllerBehavior::Start()
{
    if (!GetOwner() || !GetOwner()->GetScene()) return;
    auto* current = Game::StageDecoration();
    if (current && current != this && current->GetEnable()) { SetEnable(false); return; }
    GameControllerLocator::s_stageDecorationController = this;
    Rebuild();
}

void StageDecorationControllerBehavior::DestroyGeneratedObjects()
{
    auto* scene = GetOwner() ? GetOwner()->GetScene() : nullptr;
    if (scene) {
        for (const auto& generated : m_objects) {
            auto* object = scene->GetGameObjectByID(generated.id);
            if (!object || object->GetName() != generated.name) continue;
            object->SetActive(false);
            object->Destroy();
        }
    }
    m_objects.clear();
}

void StageDecorationControllerBehavior::Rebuild()
{
    auto* scene = GetOwner() ? GetOwner()->GetScene() : nullptr;
    if (!scene) return;
    DestroyGeneratedObjects();
    m_rebuildRequested = false;
    m_revision = m_settingsAsset ? m_settingsAsset->GetRevision() : 0;
    m_status.clear();
    if (!m_settingsAsset) { m_status = "No decoration settings asset"; return; }
    for (auto settings : m_settingsAsset->GetData().objects) {
        if (!settings.enabled) continue;
        StageDecorationSettings::Sanitize(settings);
        std::error_code error;
        if (settings.modelPath.empty() || !std::filesystem::is_regular_file(settings.modelPath, error)) {
            m_status += "Missing model: " + settings.name + "\n";
            continue;
        }
        auto* resource = MODEL_REPOSITORY->GetModel(settings.modelPath);
        if (!resource) { m_status += "Model load failed: " + settings.name + "\n"; continue; }
        // 選択肢はLit/Unlitだけ。頂点形式に合うシェーダーを内部で選ぶ。
        const bool skinned = resource->vertexType == ModelResource::VertexType::Skinned;
        const bool lit = settings.shader == StageDecorationSettings::Shader::Lit;
        const auto shaderType = skinned ? (lit ? ShaderBase::SkinnedLit : ShaderBase::SkinnedUnlit)
                                       : (lit ? ShaderBase::Lit : ShaderBase::Unlit);
        auto* shader = SHADER_REPOSITORY->GetShaderProgramResource(shaderType);
        if (!shader) { m_status += "Shader unavailable: " + settings.name + "\n"; continue; }
        auto* object = scene->CreateGameObject();
        const auto name = settings.name + "_Decoration_" + std::to_string(++m_serial);
        object->SetName(name);
        auto* transform = object->AddComponent<TransformComponent>();
        transform->SetPosition(settings.position);
        transform->SetScaling(settings.scale);
        transform->SetEulerRawAngle({DirectX::XMConvertToRadians(settings.rotationDegrees.x),
            DirectX::XMConvertToRadians(settings.rotationDegrees.y), DirectX::XMConvertToRadians(settings.rotationDegrees.z)});
        const auto initial = transform->GetRotation();
        auto* model = object->AddComponent<ModelComponent>();
        model->SetModelResource(resource);
        for (auto& material : model->GetMaterialSlots()) {
            material.isOverrideShaderProgram = true;
            material.overrideShaderProgram = shader;
        }
        m_objects.push_back({object->GetID(), name, initial, settings.rotationAxis,
            settings.rotationEnabled ? settings.rotationSpeedDegrees : 0});
    }
    if (m_status.empty()) m_status = "Decorations generated";
}

void StageDecorationControllerBehavior::Update()
{
    if (Game::StageDecoration() != this || !GetOwner()) return;
    auto* scene = GetOwner()->GetScene();
    if (!scene) return;
    if (m_rebuildRequested || m_revision != (m_settingsAsset ? m_settingsAsset->GetRevision() : 0)) Rebuild();
    const float dt = m_settingsAsset && m_settingsAsset->GetData().useUnscaledTime
        ? FPS_GetUnscaledDeltaTime() : FPS_GetDeltaTime();
    if (!std::isfinite(dt) || dt < 0) return;
    std::erase_if(m_objects, [&](GeneratedObject& generated) {
        auto* object = scene->GetGameObjectByID(generated.id);
        if (!object || object->GetName() != generated.name) return true;
        if (!object->GetActive()) return false;
        auto* transform = object->GetComponent<TransformComponent>();
        if (!transform || !transform->GetEnable() || generated.speed == 0 || dt == 0) return false;
        generated.angle = std::remainder(generated.angle + generated.speed * dt, 360.0f);
        transform->SetRotation(StageDecorationSettings::ResolveRotation(generated.initialRotation, generated.axis, generated.angle));
        return false;
    });
}

void StageDecorationControllerBehavior::DrawComponentInspector()
{
    ImGui::PushID(this);
    ImGui::TextUnformatted("Settings: asset/Data/stage_decoration_settings.data.json");
    ImGui::Text("Generated: %d", static_cast<int>(m_objects.size()));
    ImGui::TextWrapped("%s", m_status.c_str());
    if (ImGui::Button("Rebuild Decorations")) RequestRebuild();
    ImGui::PopID();
}
