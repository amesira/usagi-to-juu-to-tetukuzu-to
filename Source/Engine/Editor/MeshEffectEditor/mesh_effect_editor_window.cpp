//===================================================
// File  ：Engine/Editor/MeshEffectEditor/mesh_effect_editor_window.cpp
//===================================================
#include "mesh_effect_editor_window.h"

#include <algorithm>
#include <cstring>

#include "Engine/Editor/editor_context.h"
#include "Engine/render_view.h"

namespace {
    const std::filesystem::path MeshEffectAssetDirectory = "asset/MeshEffect";
}

MeshEffectEditorWindow::MeshEffectEditorWindow(EditorContext* editorContext)
    : IEditorWindow(editorContext)
    , m_preview(editorContext)
{
    strncpy_s(
        m_pathBuffer.data(),
        m_pathBuffer.size(),
        "asset/MeshEffect/new_mesh_effect.mesh_effect.json",
        _TRUNCATE);
    RefreshAssetList();
}

void MeshEffectEditorWindow::Draw()
{
    m_preview.GeneratePreviewObject(m_document.GetEditingDesc());
    m_preview.Update();

    const float statusBarHeight = ImGui::GetFrameHeightWithSpacing();
    ImGui::BeginChild("MeshEffectMainContent", ImVec2(0.0f, -statusBarHeight), false);

    DrawToolbar();
    ImGui::Separator();

    if (ImGui::BeginTable(
        "MeshEffectEditorLayout",
        3,
        ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV)) {
        ImGui::TableSetupColumn("Assets", ImGuiTableColumnFlags_WidthFixed, 230.0f);
        ImGui::TableSetupColumn("Preview", ImGuiTableColumnFlags_WidthStretch, 1.3f);
        ImGui::TableSetupColumn("Parameters", ImGuiTableColumnFlags_WidthStretch, 1.0f);
        ImGui::TableNextRow();

        ImGui::TableSetColumnIndex(0);
        DrawAssetList();
        ImGui::TableSetColumnIndex(1);
        DrawPreview();
        ImGui::TableSetColumnIndex(2);
        DrawParameters();
        ImGui::EndTable();
    }

    ImGui::EndChild();
    ImGui::Separator();
    DrawStatusBar();
}

void MeshEffectEditorWindow::OnWindowClosed()
{
    m_preview.Cleanup();
}

void MeshEffectEditorWindow::OnSceneDestroyed()
{
    m_preview.OnSceneDestroyed();
}

void MeshEffectEditorWindow::DrawToolbar()
{
    if (ImGui::Button("New")) {
        m_document.New();
        strncpy_s(
            m_pathBuffer.data(),
            m_pathBuffer.size(),
            "asset/MeshEffect/new_mesh_effect.mesh_effect.json",
            _TRUNCATE);
        m_preview.Apply(m_document.GetEditingDesc(), true);
    }
    ImGui::SameLine();

    if (ImGui::Button("Save")) {
        m_document.Save(m_liveApplyToScene);
    }
    ImGui::SameLine();

    if (ImGui::Button("Save As")) {
        std::filesystem::path path(m_pathBuffer.data());
        if (m_document.SaveAs(path, m_liveApplyToScene)) {
            RefreshAssetList();
            SyncPathBuffer();
        }
    }
    ImGui::SameLine();

    if (ImGui::Button("Refresh Assets")) RefreshAssetList();
    ImGui::SameLine();
    ImGui::Checkbox("Live Apply to Scene", &m_liveApplyToScene);

    ImGui::Checkbox("Auto Save", &m_autoSave);
    ImGui::SameLine();
    ImGui::Text("%d/100", m_autoSaveFrameCounter);
    if (m_autoSave) {
        ++m_autoSaveFrameCounter;
        if (m_autoSaveFrameCounter >= 100) {
            m_autoSaveFrameCounter = 0;
            if (m_document.IsDirty()) {
                m_document.Save(m_liveApplyToScene);
            }
        }
    }

    ImGui::SetNextItemWidth(-1.0f);
    ImGui::InputText(
        "##MeshEffectAssetPath",
        m_pathBuffer.data(),
        m_pathBuffer.size());
}

void MeshEffectEditorWindow::DrawAssetList()
{
    ImGui::TextUnformatted("Mesh Effect Assets");
    ImGui::Separator();

    ImGui::BeginChild("MeshEffectAssetList", { 0.0f, 0.0f }, false);
    for (const std::filesystem::path& path : m_assetPaths) {
        const bool selected = m_document.HasAssetPath() &&
            m_document.GetAssetPath().lexically_normal() == path.lexically_normal();
        if (ImGui::Selectable(path.filename().string().c_str(), selected)) {
            OpenMeshEffectAsset(path);
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", path.generic_string().c_str());
        }
    }

    if (m_assetPaths.empty()) {
        ImGui::TextDisabled("No mesh effect assets in");
        ImGui::TextWrapped("%s", MeshEffectAssetDirectory.generic_string().c_str());
    }
    ImGui::EndChild();
}

void MeshEffectEditorWindow::DrawPreview()
{
    ImGui::TextUnformatted("Scene Preview");
    ImGui::SameLine();
    if (ImGui::Button("Play")) m_preview.Play();
    ImGui::SameLine();
    if (ImGui::Button("Pause")) m_preview.Pause();
    ImGui::SameLine();
    if (ImGui::Button("Stop")) m_preview.Stop();
    ImGui::SameLine();
    ImGui::TextDisabled(m_preview.IsPlaying() ? "Playing" : "Stopped");

    ImGui::BeginChild("MeshEffectPreview", { 0.0f, 0.0f }, true);
    const RenderView* renderView = m_editorContext->sceneRenderView;
    if (renderView && renderView->colorBufferSRV) {
        ImVec2 size = ImGui::GetContentRegionAvail();
        if (size.y > 0.0f && size.x / size.y > renderView->aspectRatio) {
            size.x = size.y * renderView->aspectRatio;
        }
        else if (size.x > 0.0f) {
            size.y = size.x / renderView->aspectRatio;
        }
        ImGui::Image(
            reinterpret_cast<ImTextureID>(renderView->colorBufferSRV.Get()),
            size);
    }
    else {
        ImGui::TextDisabled("Scene RenderView is not available.");
    }
    ImGui::EndChild();
}

void MeshEffectEditorWindow::DrawParameters()
{
    ImGui::TextUnformatted("Parameters");
    ImGui::Separator();
    ImGui::BeginChild("MeshEffectParameters", { 0.0f, 0.0f }, false);
    if (m_parameterPanel.Draw(m_document.GetEditingDesc())) {
        m_document.MarkDirty();
        m_preview.Apply(m_document.GetEditingDesc(), true);
    }
    ImGui::EndChild();
}

void MeshEffectEditorWindow::DrawStatusBar()
{
    const float statusBarHeight = ImGui::GetFrameHeightWithSpacing();
    const float windowBottom = ImGui::GetWindowContentRegionMax().y;
    ImGui::SetCursorPosY(windowBottom - statusBarHeight);
    ImGui::Separator();

    const std::string path = m_document.HasAssetPath()
        ? m_document.GetAssetPath().generic_string()
        : "Untitled";
    ImGui::Text("%s%s", path.c_str(), m_document.IsDirty() ? " *" : "");
    ImGui::SameLine();
    ImGui::TextDisabled("| %s", m_document.GetStatusMessage().c_str());
}

void MeshEffectEditorWindow::RefreshAssetList()
{
    m_assetPaths.clear();
    std::error_code error;
    if (!std::filesystem::exists(MeshEffectAssetDirectory, error)) return;

    std::filesystem::recursive_directory_iterator iterator(
        MeshEffectAssetDirectory,
        std::filesystem::directory_options::skip_permission_denied,
        error);
    const std::filesystem::recursive_directory_iterator end;

    while (!error && iterator != end) {
        if (iterator->is_regular_file(error) &&
            iterator->path().extension() == ".json" &&
            iterator->path().stem().extension() == ".mesh_effect") {
            m_assetPaths.push_back(iterator->path().lexically_normal());
        }
        iterator.increment(error);
    }

    std::sort(m_assetPaths.begin(), m_assetPaths.end());
}

void MeshEffectEditorWindow::SyncPathBuffer()
{
    const std::string path = m_document.GetAssetPath().generic_string();
    strncpy_s(m_pathBuffer.data(), m_pathBuffer.size(), path.c_str(), _TRUNCATE);
}

bool MeshEffectEditorWindow::OpenMeshEffectAsset(const std::filesystem::path& assetPath)
{
    if (!m_document.Open(assetPath)) return false;
    SyncPathBuffer();
    m_preview.Apply(m_document.GetEditingDesc(), true);
    return true;
}
