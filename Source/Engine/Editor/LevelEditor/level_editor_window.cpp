//===================================================
// File  ：_/Editor/LevelEditor/level_editor_window.cpp
// Date  ：2026/09/02
// Author：Miu Kitamura
//===================================================
#include "level_editor_window.h"

#include "Engine/Editor/editor_context.h"
#include "Engine/Asset/LevelAsset/level_schema.h"
#include "Engine/Component/collider_component.h"
#include "Engine/Component/level_object_component.h"
#include "Engine/Component/model_component.h"
#include "Engine/Component/transform_component.h"
#include "Engine/Core/game_object.h"
#include "Engine/Core/scene_interface.h"
#include "Engine/engine_service_locator.h"
#include "Game/Factory/level_object_factory.h"

#include <algorithm>
#include <cstring>
#include <typeinfo>

using namespace DirectX;

namespace
{
    // ImGuiのInputTextでstd::stringを扱うためのラッパー関数
    bool InputString(const char* label, std::string& value)
    {
        char buffer[512]{};
        strncpy_s(buffer, value.c_str(), _TRUNCATE);
        if (!ImGui::InputText(label, buffer, sizeof(buffer))) return false;
        value = buffer;
        return true;
    }
}

LevelEditorWindow::LevelEditorWindow(EditorContext* editorContext)
    : IEditorWindow(editorContext)
{
    m_document.New();
    strncpy_s(m_pathBuffer.data(), m_pathBuffer.size(),
        "asset/Level/new_level.level.json", _TRUNCATE);
}

void LevelEditorWindow::Draw()
{
    // シーン自体がリロードされた場合は、開いているLevelAssetを新しいシーンへ復元する
    // FIX: アセットを読み込んで生成するのはシーン内でするべきかも
    if (m_editorContext->scene && m_editorContext->scene != m_appliedScene)
    {
        m_appliedScene = m_editorContext->scene;
        LevelObjectFactory::CreateLevel(m_appliedScene, m_document.GetAsset());
    }

    DrawToolbar();
    ImGui::Separator();

    // === レベルオブジェクトとランタイムオブジェクトのタブを表示 ===
    if (ImGui::BeginTabBar("LevelEditorTabs"))
    {
        if (ImGui::BeginTabItem("Level Objects"))
        {
            DrawLevelObjectsTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Runtime Objects"))
        {
            DrawRuntimeObjectsTab();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::Separator();
    if (m_document.IsDirty()) ImGui::TextColored({ 1.0f, 0.75f, 0.2f, 1.0f }, "Unsaved changes");
    ImGui::SameLine();
    ImGui::TextDisabled("%s", m_document.GetStatusMessage().c_str());
}

#pragma region エディター内部描画
void LevelEditorWindow::DrawToolbar()
{
    ImGui::SetNextItemWidth(-360.0f);
    ImGui::InputText("##LevelAssetPath", m_pathBuffer.data(), m_pathBuffer.size());
    ImGui::SameLine();
    if (ImGui::Button("New"))
    {
        DestroyLevelObjects();
        m_document.New();
        m_selectedLevelObjectId.clear();
        m_editorContext->selectedObject = nullptr;
    }
    ImGui::SameLine();
    if (ImGui::Button("Load"))
    {
        if (m_document.Load(m_pathBuffer.data()))
        {
            SyncPathBuffer();
            RebuildLevelObjects();
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Save")) m_document.Save();
    ImGui::SameLine();
    if (ImGui::Button("Save As"))
    {
        if (m_document.SaveAs(m_pathBuffer.data())) SyncPathBuffer();
    }
    ImGui::SameLine();
    if (ImGui::Button("Reload"))
    {
        if (m_document.Reload())
        {
            SyncPathBuffer();
            RebuildLevelObjects();
        }
    }
}

void LevelEditorWindow::DrawLevelObjectsTab()
{
    if (ImGui::BeginTable("LevelObjectLayout", 2,
        ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV))
    {
        ImGui::TableSetupColumn("Hierarchy", ImGuiTableColumnFlags_WidthFixed, 280.0f);
        ImGui::TableSetupColumn("Inspector", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        DrawLevelHierarchy();
        ImGui::TableSetColumnIndex(1);
        DrawLevelInspector();
        ImGui::EndTable();
    }
}

void LevelEditorWindow::DrawRuntimeObjectsTab()
{
    if (ImGui::BeginTable("RuntimeObjectLayout", 2,
        ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV))
    {
        ImGui::TableSetupColumn("Hierarchy", ImGuiTableColumnFlags_WidthFixed, 280.0f);
        ImGui::TableSetupColumn("Inspector", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        DrawRuntimeHierarchy();
        ImGui::TableSetColumnIndex(1);
        DrawRuntimeInspector();
        ImGui::EndTable();
    }
}

void LevelEditorWindow::DrawLevelHierarchy()
{
    if (ImGui::Button("Add"))
    {
        LevelObjectData& data = m_document.AddObject();
        if (m_editorContext->scene) LevelObjectFactory::CreateObject(m_editorContext->scene, data);
        SelectLevelObject(data.id);
    }
    ImGui::SameLine();
    const bool canDelete = !m_selectedLevelObjectId.empty();
    ImGui::BeginDisabled(!canDelete);
    if (ImGui::Button("Delete"))
    {
        if (GameObject* object = FindLevelObject(m_selectedLevelObjectId))
        {
            object->SetActive(false);
            object->Destroy();
        }
        m_document.RemoveObject(m_selectedLevelObjectId);
        m_selectedLevelObjectId.clear();
        m_editorContext->selectedObject = nullptr;
    }
    ImGui::EndDisabled();
    ImGui::Separator();

    ImGui::BeginChild("LevelHierarchyList", { 0.0f, 0.0f });
    for (const LevelObjectData& data : m_document.GetAsset().GetData().objects)
    {
        ImGui::PushID(data.id.c_str());
        if (ImGui::Selectable(data.name.c_str(), data.id == m_selectedLevelObjectId))
            SelectLevelObject(data.id);
        ImGui::PopID();
    }
    ImGui::EndChild();
}

void LevelEditorWindow::DrawLevelInspector()
{
    LevelObjectData* data = m_document.FindObject(m_selectedLevelObjectId);
    if (!data)
    {
        ImGui::TextDisabled("Select a level object");
        return;
    }

    bool changed = false;
    ImGui::BeginChild("LevelObjectInspector");
    ImGui::TextDisabled("ID: %s", data->id.c_str());
    changed |= InputString("Name", data->name);

    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
    {
        changed |= ImGui::DragFloat3("Position", &data->transform.position.x, 0.1f);
        changed |= ImGui::DragFloat3("Rotation", &data->transform.rotationDegrees.x, 0.25f);
        changed |= ImGui::DragFloat3("Scale", &data->transform.scale.x, 0.1f);
    }

    if (ImGui::CollapsingHeader("Collider", ImGuiTreeNodeFlags_DefaultOpen))
    {
        int type = static_cast<int>(data->collider.type);
        const char* types[] = { "None", "Box", "Sphere" };
        if (ImGui::Combo("Type", &type, types, IM_ARRAYSIZE(types)))
        {
            data->collider.type = static_cast<LevelColliderType>(type);
            changed = true;
        }
        changed |= ImGui::DragFloat3("Center", &data->collider.center.x, 0.1f);
        if (data->collider.type == LevelColliderType::Box)
            changed |= ImGui::DragFloat3("Size", &data->collider.boxSize.x, 0.1f, 0.0f);
        else if (data->collider.type == LevelColliderType::Sphere)
            changed |= ImGui::DragFloat("Radius", &data->collider.sphereRadius, 0.05f, 0.0f);
    }

    if (ImGui::CollapsingHeader("Model", ImGuiTreeNodeFlags_DefaultOpen))
        changed |= InputString("Model Path", data->modelPath);

    if (ImGui::CollapsingHeader("Materials", ImGuiTreeNodeFlags_DefaultOpen))
    {
        for (size_t i = 0; i < data->materialNames.size(); ++i)
        {
            ImGui::PushID(static_cast<int>(i));
            changed |= InputString("Material", data->materialNames[i]);
            ImGui::SameLine();
            if (ImGui::Button("Remove"))
            {
                data->materialNames.erase(data->materialNames.begin() + i);
                changed = true;
                ImGui::PopID();
                break;
            }
            ImGui::PopID();
        }
        if (ImGui::Button("Add Material"))
        {
            data->materialNames.emplace_back();
            changed = true;
        }
    }
    ImGui::EndChild();

    if (changed)
    {
        m_document.MarkDirty();
        ApplyObjectData(FindLevelObject(data->id), *data);
    }
}

void LevelEditorWindow::DrawRuntimeHierarchy()
{
    ImGui::TextDisabled("Objects not owned by the level asset");
    ImGui::Separator();
    ImGui::BeginChild("RuntimeHierarchyList");
    if (m_editorContext->scene)
    {
        for (GameObject& object : m_editorContext->scene->GetGameObjects())
        {
            if (object.GetID() == static_cast<unsigned int>(-1) || !object.GetActive()) continue;
            if (object.GetComponent<LevelObjectComponent>()) continue;
            ImGui::PushID(&object);
            if (ImGui::Selectable(object.GetName().c_str(), object.GetID() == m_selectedRuntimeObjectId))
            {
                m_selectedRuntimeObjectId = object.GetID();
                m_editorContext->selectedObject = &object;
            }
            ImGui::PopID();
        }
    }
    ImGui::EndChild();
}

void LevelEditorWindow::DrawRuntimeInspector()
{
    GameObject* object = FindRuntimeObject();
    if (!object)
    {
        ImGui::TextDisabled("Select a runtime object");
        return;
    }
    ImGui::Text("Name: %s", object->GetName().c_str());
    ImGui::Text("ID: %u", object->GetID());
    ImGui::Text("Tag: %s", object->GetTag().c_str());
    bool active = object->GetActive();
    if (ImGui::Checkbox("Active", &active)) object->SetActive(active);
    ImGui::Separator();
    ImGui::TextUnformatted("Components");
    for (Component* component : object->GetAllComponents())
        ImGui::BulletText("%s%s", typeid(*component).name(), component->GetEnable() ? "" : " (Disabled)");

    // BehaviorComponentの詳細表示
    {
        auto behaviorComponents = object->GetBehaviorComponents();

        ImGui::Separator();
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.18f, 0.23f, 0.18f, 1.0f));
        ImGui::TextUnformatted("Behavior Components");

        ImGui::BeginGroup();
        for (BehaviorComponent* behavior : behaviorComponents)
        {
            behavior->DrawComponentInspector();
        }
        ImGui::EndGroup();
        ImGui::PopStyleColor();
    }
}
#pragma endregion

#pragma region ユーティリティ関数
GameObject* LevelEditorWindow::FindLevelObject(const std::string& id) const
{
    if (!m_editorContext->scene || id.empty()) return nullptr;
    for (GameObject& object : m_editorContext->scene->GetGameObjects())
    {
        LevelObjectComponent* marker = object.GetComponent<LevelObjectComponent>();
        if (marker && object.GetActive() && marker->GetLevelObjectId() == id) return &object;
    }
    return nullptr;
}

GameObject* LevelEditorWindow::FindRuntimeObject() const
{
    if (!m_editorContext->scene || m_selectedRuntimeObjectId == static_cast<unsigned int>(-1)) return nullptr;
    GameObject* object = m_editorContext->scene->GetGameObjectByID(m_selectedRuntimeObjectId);
    return object && object->GetActive() ? object : nullptr;
}

/// @brief GameObjectにLevelObjectDataの内容を適用する
bool LevelEditorWindow::ApplyObjectData(GameObject* object, const LevelObjectData& data)
{
    if (!object) return false;
    object->SetName(data.name);
    if (TransformComponent* transform = object->GetComponent<TransformComponent>())
    {
        transform->SetPosition(data.transform.position);
        transform->SetEulerRawAngle({ XMConvertToRadians(data.transform.rotationDegrees.x),
            XMConvertToRadians(data.transform.rotationDegrees.y),
            XMConvertToRadians(data.transform.rotationDegrees.z) });
        transform->SetScaling(data.transform.scale);
    }
    if (BoxColliderComponent* box = object->GetComponent<BoxColliderComponent>())
    {
        box->SetCenter(data.collider.center);
        box->SetScale(data.collider.boxSize);
        box->SetEnable(data.collider.type == LevelColliderType::Box);
    }
    if (SphereColliderComponent* sphere = object->GetComponent<SphereColliderComponent>())
    {
        sphere->SetCenter(data.collider.center);
        sphere->SetRadius(data.collider.sphereRadius);
        sphere->SetEnable(data.collider.type == LevelColliderType::Sphere);
    }
    ModelComponent* model = object->GetComponent<ModelComponent>();
    if (model)
    {
        if (data.modelPath.empty()) model->SetEnable(false);
        else if (ModelResource* resource = MODEL_REPOSITORY->GetModel(data.modelPath))
        {
            model->SetModelResource(resource);
            model->SetEnable(true);
            auto& slots = model->GetMaterialSlots();
            const size_t count = (std::min)(slots.size(), data.materialNames.size());
            for (size_t i = 0; i < count; ++i)
                if (!data.materialNames[i].empty())
                    slots[i].materialResource = MATERIAL_REPOSITORY->GetMaterial(data.materialNames[i]);
        }
    }
    return true;
}

/// @brief レベルオブジェクトを再構築する
void LevelEditorWindow::RebuildLevelObjects()
{
    DestroyLevelObjects();
    m_selectedLevelObjectId.clear();
    m_editorContext->selectedObject = nullptr;
    if (m_editorContext->scene)
    {
        m_appliedScene = m_editorContext->scene;
        LevelObjectFactory::CreateLevel(m_editorContext->scene, m_document.GetAsset());
    }
}

void LevelEditorWindow::DestroyLevelObjects()
{
    if (!m_editorContext->scene) return;
    for (GameObject& object : m_editorContext->scene->GetGameObjects())
    {
        if (!object.GetComponent<LevelObjectComponent>()) continue;
        object.SetActive(false);
        object.Destroy();
    }
}

void LevelEditorWindow::SelectLevelObject(const std::string& id)
{
    m_selectedLevelObjectId = id;
    m_editorContext->selectedObject = FindLevelObject(id);
}
#pragma endregion

/// @brief m_documentのアセットパスをm_pathBufferに同期する
void LevelEditorWindow::SyncPathBuffer()
{
    strncpy_s(m_pathBuffer.data(), m_pathBuffer.size(),
        m_document.GetAssetPath().generic_string().c_str(), _TRUNCATE);
}

/// @brief シーンが破棄されたときの処理
void LevelEditorWindow::OnSceneDestroyed()
{
    m_selectedLevelObjectId.clear();
    m_selectedRuntimeObjectId = static_cast<unsigned int>(-1);
    m_appliedScene = nullptr;
}
