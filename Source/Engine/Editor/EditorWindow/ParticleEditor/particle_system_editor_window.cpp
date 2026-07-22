//===================================================
// File  ：Engine/Editor/EditorWindow/ParticleEditor
//          /particle_system_editor_window.cpp
// Date  ：2026/07/21
// Author：Miu Kitamura
// 
// ・ParticleEditor全体を管理するウィンドウクラス
//===================================================
#include "particle_system_editor_window.h"

#include <algorithm>
#include <cstring>

#include "Engine/Editor/editor_context.h"
#include "Engine/Framework/Component/particle_system_component.h"
#include "Engine/render_view.h"

namespace
{
    // パーティクルアセットのディレクトリ
    const std::filesystem::path PARTICLE_ASSET_DIRECTORY = "asset/Particle";
}

ParticleSystemEditorWindow::ParticleSystemEditorWindow(EditorContext* editorContext)
    : IEditorWindow(editorContext)
    , m_preview(editorContext)
{
    // デフォルトのパーティクルアセットを作成し、パスバッファを初期化する
    strncpy_s(m_pathBuffer.data(), m_pathBuffer.size(),
        "asset/Particle/new_particle.json", _TRUNCATE);

    // パーティクルアセットのリストを更新する
    RefreshAssetList();
}

/// @brief ParticleEditorウィンドウの描画
void ParticleSystemEditorWindow::Draw()
{
    // パーティクルアセットのプレビューを作成し、更新する
    m_preview.EnsureCreated(m_document.GetEditingDesc());
    m_preview.Update();

    // ツールバーを描画
    DrawToolbar();
    ImGui::Separator();

    // レイアウトを3列のテーブルで描画する
    if (ImGui::BeginTable("ParticleEditorLayout", 3,
        ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV))
    {
        // 左から順に、アセットリスト、プレビュー、パラメータ編集パネルの列を設定する
        ImGui::TableSetupColumn("Assets", ImGuiTableColumnFlags_WidthFixed, 230.0f);
        ImGui::TableSetupColumn("Preview", ImGuiTableColumnFlags_WidthStretch, 1.3f);
        ImGui::TableSetupColumn("Parameters", ImGuiTableColumnFlags_WidthStretch, 1.0f);
        ImGui::TableNextRow();

        // 各列に対応する描画関数を呼び出す
        ImGui::TableSetColumnIndex(0);
        DrawAssetList();
        ImGui::TableSetColumnIndex(1);
        DrawPreview();
        ImGui::TableSetColumnIndex(2);
        DrawParameters();
        ImGui::EndTable();
    }

    // ステータスバーを描画する
    ImGui::Separator();
    DrawStatusBar();
}

/// @brief ParticleEditorウィンドウが閉じられたときの処理
/// FIX: IEditorWindowにも追加してオーバーライドした方が良さそう
void ParticleSystemEditorWindow::OnWindowClosed()
{
    m_preview.Cleanup();
}

#pragma region ParticleEditorの内部描画関数
/// @brief ParticleEditorのツールバーを描画する
void ParticleSystemEditorWindow::DrawToolbar()
{
    if (ImGui::Button("New"))
    {
        m_document.New();
        strncpy_s(m_pathBuffer.data(), m_pathBuffer.size(),
            "asset/Particle/new_particle.json", _TRUNCATE);
        m_preview.Apply(m_document.GetEditingDesc(), true);
    }
    ImGui::SameLine();
    if (ImGui::Button("Save")) m_document.Save();
    ImGui::SameLine();
    if (ImGui::Button("Save As"))
    {
        if (m_document.SaveAs(std::filesystem::path(m_pathBuffer.data()))) RefreshAssetList();
    }
    ImGui::SameLine();
    if (ImGui::Button("Reload"))
    {
        if (m_document.Reload()) m_preview.Apply(m_document.GetEditingDesc(), true);
    }
    ImGui::SameLine();
    if (ImGui::Button("Refresh Assets")) RefreshAssetList();

    ImGui::SetNextItemWidth(-1.0f);
    ImGui::InputText("##ParticleAssetPath", m_pathBuffer.data(), m_pathBuffer.size());
}

/// @brief ParticleEditorのアセットリストを描画する
void ParticleSystemEditorWindow::DrawAssetList()
{
    ImGui::TextUnformatted("Particle Assets");
    ImGui::Separator();
    ImGui::BeginChild("ParticleAssetList", { 0.0f, 0.0f }, false);
    for (const std::filesystem::path& path : m_assetPaths)
    {
        const bool selected = m_document.HasAssetPath() &&
            m_document.GetAssetPath().lexically_normal() == path.lexically_normal();
        if (ImGui::Selectable(path.filename().string().c_str(), selected)) OpenAsset(path);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", path.generic_string().c_str());
    }
    if (m_assetPaths.empty())
    {
        ImGui::TextDisabled("No particle assets in");
        ImGui::TextWrapped("%s", PARTICLE_ASSET_DIRECTORY.generic_string().c_str());
    }
    ImGui::EndChild();
}

/// @brief ParticleEditorのプレビューを描画する
void ParticleSystemEditorWindow::DrawPreview()
{
    ImGui::TextUnformatted("Scene Preview");
    ImGui::SameLine();
    if (ImGui::Button("Play")) m_preview.Play();
    ImGui::SameLine();
    if (ImGui::Button("Pause")) m_preview.Pause();
    ImGui::SameLine();
    if (ImGui::Button("Stop")) m_preview.Stop();
    ImGui::SameLine();
    if (ImGui::Button("Restart")) m_preview.Restart();

    if (ParticleSystemComponent* particleSystem = m_preview.GetParticleSystem())
    {
        ImGui::SameLine();
        ImGui::TextDisabled("Particles: %zu", particleSystem->Particles().size());
    }

    ImGui::BeginChild("ParticlePreview", { 0.0f, 0.0f }, true);
    const RenderView* renderView = m_editorContext->sceneRenderView;
    if (renderView && renderView->colorBufferSRV)
    {
        ImVec2 size = ImGui::GetContentRegionAvail();
        if (size.y > 0.0f && size.x / size.y > renderView->aspectRatio)
            size.x = size.y * renderView->aspectRatio;
        else if (size.x > 0.0f)
            size.y = size.x / renderView->aspectRatio;
        ImGui::Image(reinterpret_cast<ImTextureID>(renderView->colorBufferSRV.Get()), size);
    }
    else
    {
        ImGui::TextDisabled("Scene RenderView is not available.");
    }
    ImGui::EndChild();
}

/// @brief ParticleEditorのパラメータ編集パネルを描画する
void ParticleSystemEditorWindow::DrawParameters()
{
    ImGui::TextUnformatted("Parameters");
    ImGui::Separator();
    ImGui::BeginChild("ParticleParameters", { 0.0f, 0.0f }, false);
    if (m_parameterPanel.Draw(m_document.GetEditingDesc()))
    {
        m_document.MarkDirty();
        m_preview.Apply(m_document.GetEditingDesc(), true);
    }
    ImGui::EndChild();
}

/// @brief ParticleEditorのステータスバーを描画する
void ParticleSystemEditorWindow::DrawStatusBar()
{
    const std::string path = m_document.HasAssetPath()
        ? m_document.GetAssetPath().generic_string()
        : "Untitled";
    ImGui::Text("%s%s", path.c_str(), m_document.IsDirty() ? " *" : "");
    ImGui::SameLine();
    ImGui::TextDisabled("| %s", m_document.GetStatusMessage().c_str());
}
#pragma endregion

/// @brief パーティクルアセットのリストを更新する
void ParticleSystemEditorWindow::RefreshAssetList()
{
    m_assetPaths.clear();
    std::error_code error;
    if (!std::filesystem::exists(PARTICLE_ASSET_DIRECTORY, error)) return;

    std::filesystem::recursive_directory_iterator iterator(
        PARTICLE_ASSET_DIRECTORY, std::filesystem::directory_options::skip_permission_denied, error);
    const std::filesystem::recursive_directory_iterator end;
    while (!error && iterator != end)
    {
        if (iterator->is_regular_file(error) && iterator->path().extension() == ".json")
            m_assetPaths.push_back(iterator->path().lexically_normal());
        iterator.increment(error);
    }
    std::sort(m_assetPaths.begin(), m_assetPaths.end());
}

/// @brief パーティクルアセットのパスバッファを同期する
void ParticleSystemEditorWindow::SyncPathBuffer()
{
    const std::string path = m_document.GetAssetPath().generic_string();
    strncpy_s(m_pathBuffer.data(), m_pathBuffer.size(), path.c_str(), _TRUNCATE);
}

/// @brief 指定されたパスのパーティクルアセットを開く
bool ParticleSystemEditorWindow::OpenAsset(const std::filesystem::path& path)
{
    if (!m_document.Open(path)) return false;
    SyncPathBuffer();
    m_preview.Apply(m_document.GetEditingDesc(), true);
    return true;
}