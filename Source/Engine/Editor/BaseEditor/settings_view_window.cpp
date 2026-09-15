//===================================================
// settings_view_window.cpp
// 
// Author：Miu Kitamura
// Date  ：2026/04/30
//===================================================
#include "settings_view_window.h"
#include "Engine/Editor/editor_context.h"
#include "Engine/Editor/scene_view_camera_state.h"
#include "Engine/Editor/Schema/field_editor.h"
#include "Engine/Asset/EnvironmentAsset/environment_asset.h"
#include "Engine/Asset/EnvironmentAsset/environment_lighting_schema.h"
#include "Engine/Asset/EnvironmentAsset/environment_post_process_schema.h"
#include "Engine/engine_service_locator.h"

namespace {
    const std::filesystem::path DefaultEnvironmentPath =
        "asset/Environment/default.environment.json";
}


void SettingsViewWindow::Draw()
{
    if (!m_editorContext->environmentAsset || !m_editorContext->sceneViewCamera) {
        ImGui::TextDisabled("No active scene.");
        return;
    }

    if (ImGui::Button("Save Environment")) {
        EnvironmentAsset& asset = *m_editorContext->environmentAsset;
        if (asset.GetHeader().m_name.empty()) {
            AssetHeader header;
            header.m_type = "Environment";
            header.m_formatVersion = 1;
            header.m_name = "Default Environment";
            asset.SetHeader(header);
        }
        if (EnvironmentAssetLoader* loader = EngineServiceLocator::EnvironmentLoader()) {
            loader->Save(DefaultEnvironmentPath, asset);
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Reload Environment")) {
        if (EnvironmentAssetLoader* loader = EngineServiceLocator::EnvironmentLoader()) {
            loader->Load(DefaultEnvironmentPath, *m_editorContext->environmentAsset);
        }
    }

    if (ImGui::BeginTabBar("Settings View Group")) {
        ImVec2 avail = ImGui::GetContentRegionAvail();

        if (ImGui::BeginTabItem("Scene Camera")) {
            DrawSceneCameraSettings();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Lighting")) {
            DrawLightingSettings();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Post Processing")) {
            DrawPostProcessingSettings();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
}

// ----------------------------- private

// シーンカメラの設定UIを描画
void SettingsViewWindow::DrawSceneCameraSettings()
{
    ImGui::Text("Scene Camera Settings");
    SceneViewCameraState& camera = *m_editorContext->sceneViewCamera;

    // カメラの位置を表示
    ImGui::DragFloat3("Position", &camera.position.x, 0.1f);
    // 注視点を表示
    ImGui::DragFloat3("At Position", &camera.atPosition.x, 0.1f);

    ImGui::Separator();

    // 視野角を表示
    ImGui::DragFloat("FOV", &camera.fovDegrees, 1.0f, 1.0f, 179.0f);
    ImGui::DragFloat("Near Clip", &camera.nearClip, 0.01f, 0.001f, camera.farClip);
    ImGui::DragFloat("Far Clip", &camera.farClip, 1.0f, camera.nearClip, 10000.0f);
}

// ライティングの設定UIを描画
void SettingsViewWindow::DrawLightingSettings()
{
    ImGui::Text("Lighting Settings");
    FieldEditor::DrawFields(
        m_editorContext->environmentAsset->GetData().lighting,
        EnvironmentLightingSchema::GetLightingSettingsSchema());
}

// PostProcessingの設定UIを描画
void SettingsViewWindow::DrawPostProcessingSettings()
{
    ImGui::Text("Post Processing Settings");
    FieldEditor::DrawFields(
        m_editorContext->environmentAsset->GetData().postProcess,
        EnvironmentPostProcessSchema::GetPostProcessSchema());
}
