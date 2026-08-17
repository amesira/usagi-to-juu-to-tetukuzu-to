//===================================================
// File  ：_/Editor/DataEditor/data_editor_window.cpp
// Date  ：2026/08/13
// Author：Miu Kitamura
// 
// ・DataEditorのウィンドウを表すクラス
//===================================================
#include "data_editor_window.h"
#include <algorithm>
#include <cstring>

#include "Engine/Editor/editor_context.h"
#include "Engine/Editor/Schema/field_editor.h"

#include "Engine/engine_service_locator.h"

namespace
{
    // パーティクルアセットのディレクトリ
    const std::filesystem::path DATA_ASSET_DIRECTORY = "asset/Data";

    // データアセットのロードを簡略化するためのマクロ
    #define DATA_LOADER EngineServiceLocator::Asset()->GetDataAssetLoader()

    /// @brief DataAssetセクションの開始
    bool BeginDataAssetSection(const char* name)
    {
        ImGui::PushID(name);
        ImGui::Separator();
        
        const ImVec4 transparent{ 0.0f, 0.0f, 0.0f, 0.0f };

        ImGui::PushStyleColor(ImGuiCol_Header, transparent);
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, transparent);
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, transparent);

        const bool open = ImGui::CollapsingHeader(name,ImGuiTreeNodeFlags_DefaultOpen);

        ImGui::PopStyleColor(3);

        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1.0f);

        return open;
    }

    /// @brief DataAssetセクションの終わり
    void EndDataAssetSection()
    {
        ImGui::PopStyleVar();
        ImGui::PopID();
    }
}

void DataEditorWindow::Draw()
{
    // === ウィンドウが開かれたときに、アセットリストを更新する ===
    if (!m_isOpened) {
        m_isOpened = true;
        RefreshAssetList();
    }

    const float statusBarHeight = ImGui::GetFrameHeightWithSpacing();
    ImGui::BeginChild("MainContent",ImVec2(0.0f, -statusBarHeight),false);

    // ツールバーを描画
    DrawToolbar();
    ImGui::Separator();

    // レイアウトを2列のテーブルで描画する
    if (ImGui::BeginTable("ParticleEditorLayout", 2,
        ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV))
    {
        // 左から順に、アセットリスト、パラメータ編集パネルの列を設定する
        ImGui::TableSetupColumn("Assets", ImGuiTableColumnFlags_WidthFixed, 230.0f);
        ImGui::TableSetupColumn("Parameters", ImGuiTableColumnFlags_WidthStretch, 1.0f);
        ImGui::TableNextRow();

        // 各列に対応する描画関数を呼び出す
        ImGui::TableSetColumnIndex(0);
        DrawAssetList();
        ImGui::TableSetColumnIndex(1);
        DrawParameters();
        ImGui::EndTable();
    }

    ImGui::EndChild();

    // ステータスバーを描画する
    ImGui::Separator();
    DrawStatusBar();
}

void DataEditorWindow::OnWindowClosed()
{
    m_isOpened = false;
}

#pragma region 内部描画

/// @brief ツールバーを描画する
void DataEditorWindow::DrawToolbar()
{
    // New
    if (ImGui::Button("New"))
    {
        m_document.New();
        strncpy_s(m_pathBuffer.data(), m_pathBuffer.size(),
            "asset/Particle/new_particle.json",
            _TRUNCATE);
    }
    ImGui::SameLine();

    // Save
    if (ImGui::Button("Save")){
        m_document.Save();
    }
    ImGui::SameLine();
    if (ImGui::Button("Save As"))
    {
        std::filesystem::path newPath(m_pathBuffer.data());

        if (m_document.SaveAs(newPath)){
            RefreshAssetList();

            // pathが修正された場合に備えて、パスバッファを同期する
            SyncPathBuffer();
        }
    }
    ImGui::SameLine();

    if (ImGui::Button("Refresh Assets")) RefreshAssetList();
    ImGui::SameLine();

    // === オートセーブ ===
    ImGui::Checkbox("Auto Save", &m_autoSave);
    ImGui::SameLine();
    ImGui::Text("%d%s%d", m_autoSaveFrameCounter, "/", 100);
    if (m_autoSave) {
        m_autoSaveFrameCounter++;

        // 100フレームごとに自動保存を行う
        if (m_autoSaveFrameCounter >= 100) {
            m_autoSaveFrameCounter = 0;
            if (m_document.IsDirty()) {
                m_document.Save();
            }
        }
    }

    // === データアセットのタイプ名とパスを表示する入力テキストボックスを描画する ===
    ImGui::SetNextItemWidth(-1.0f);
    {
        // 現在選択されているデータアセットの型名を取得する
        const std::string& selectedType = m_document.GetEditingAssetTypeName();
        // 選択されている型名が空の場合は、"Select Asset Type"と表示する
        const char* previewText = selectedType.empty() ? "Select Asset Type" : selectedType.c_str();

        // データアセットの型名を選択するコンボボックスを描画する
        if (ImGui::BeginCombo("Selected Data Asset Type", previewText))
        {
            for (const std::string& type : m_assetTypeNames)
            {
                const bool isSelected = (type == selectedType);

                if (ImGui::Selectable(type.c_str(), isSelected))
                {
                    m_document.SetEditingAssetTypeName(type);
                }

                // Comboを開いたとき、現在選択中の項目までスクロールする
                if (isSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }

            ImGui::EndCombo();
        }
    }
    ImGui::InputText("##DataAssetPath", m_pathBuffer.data(), m_pathBuffer.size());
}

/// @brief アセットのリストを描画する
void DataEditorWindow::DrawAssetList()
{
    ImGui::TextUnformatted("Data Assets");
    ImGui::Separator();

    // パーティクルアセットのリストを描画する子ウィンドウを作成する
    ImGui::BeginChild("DataAssetList", { 0.0f, 0.0f }, false);

    for (const DataAssetPathEntry& entry : m_assetPaths)
    {
        if (BeginDataAssetSection(entry.typeName.c_str()))
        {
            ImGui::Separator();

            for (const std::filesystem::path& path : entry.paths)
            {
                const bool selected = m_document.HasAssetPath() &&
                    m_document.GetAssetPath().lexically_normal() == path.lexically_normal();
                if (ImGui::Selectable(path.filename().string().c_str(), selected)) OpenAsset(path);
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", path.generic_string().c_str());
            }
        }
        
        EndDataAssetSection();

        ImGui::Spacing();
    }

    // パーティクルアセットが存在しない場合は、ディレクトリが空であることを示すメッセージを表示する
    if (m_assetPaths.empty())
    {
        ImGui::TextDisabled("No data assets in");
        ImGui::SameLine();
        ImGui::TextColored(
            ImVec4(1.0f, 1.0f, 0.2f, 1.0f),
            DATA_ASSET_DIRECTORY.generic_string().c_str()
        );
    }

    ImGui::EndChild();
}

/// @brief パラメータ編集パネルを描画する
void DataEditorWindow::DrawParameters()
{
    ImGui::TextUnformatted("Parameters");
    ImGui::Separator();
    ImGui::BeginChild("DataParameters", { 0.0f, 0.0f }, false);
    
    // 編集中のデータアセットのパラメータを描画する
    DataAsset* asset = m_document.GetEditingAsset();
    if (!asset)
    {
        ImGui::TextDisabled("No data asset selected");
        ImGui::EndChild();
        return;
    }

    bool changed = asset->DrawDataOnEditor();
    if (changed) m_document.MarkDirty();

    ImGui::EndChild();
}

/// @brief ステータスバーを描画する
void DataEditorWindow::DrawStatusBar()
{
    const float statusBarHeight = ImGui::GetFrameHeightWithSpacing();
    const float windowBottom = ImGui::GetWindowContentRegionMax().y;
    ImGui::SetCursorPosY(windowBottom - statusBarHeight);
    ImGui::Separator();

    // アセットのパスが空の場合は、"Untitled"と表示する
    const std::string path = m_document.HasAssetPath()
        ? m_document.GetAssetPath().generic_string()
        : "Untitled";
    ImGui::Text("%s%s", path.c_str(), m_document.IsDirty() ? " *" : "");
    ImGui::SameLine();
    ImGui::TextDisabled("| %s", m_document.GetStatusMessage().c_str());
}

#pragma endregion

#pragma region アセット管理

/// @brief アセットのリストを更新する
void DataEditorWindow::RefreshAssetList()
{
    m_assetPaths.clear();
    m_assetTypeNames.clear();

    std::error_code error;
    if (!std::filesystem::exists(DATA_ASSET_DIRECTORY, error)) return;

    // ディレクトリを再帰的に探索し、.data.jsonファイルのパスを取得する
    std::filesystem::recursive_directory_iterator iterator(
        DATA_ASSET_DIRECTORY, std::filesystem::directory_options::skip_permission_denied, error);
    const std::filesystem::recursive_directory_iterator end;

    while (!error && iterator != end)
    {
        if (iterator->is_regular_file(error) && iterator->path().extension() == ".json" && iterator->path().stem().extension() == ".data") 
        {
            // データアセットの型名を取得する
            std::string typeName = DATA_LOADER->GetAssetTypeName(iterator->path());

            auto it = std::find_if(m_assetPaths.begin(), m_assetPaths.end(),
                [&](const DataAssetPathEntry& entry) { return entry.typeName == typeName; });
            
            if (it != m_assetPaths.end())
            {   // 既に同じ型名のエントリが存在する場合は、パスを追加する
                it->paths.push_back(iterator->path().lexically_normal());
            }
            else
            {   // 新しい型名のエントリを作成し、パスを追加する
                m_assetPaths.push_back({ typeName, { iterator->path().lexically_normal() } });
                m_assetTypeNames.push_back(typeName); // 型名のリストにも追加する
            }
        }
        iterator.increment(error);
    }

    // アセットのパスを昇順にソートする
    for (auto& entry : m_assetPaths)
    {
        std::sort(entry.paths.begin(), entry.paths.end());
    }
}

/// @brief アセットのパスバッファを同期する
void DataEditorWindow::SyncPathBuffer()
{
    const std::string path = m_document.GetAssetPath().generic_string();
    strncpy_s(m_pathBuffer.data(), m_pathBuffer.size(), path.c_str(), _TRUNCATE);
}

/// @brief 指定されたパスのアセットを開く
bool DataEditorWindow::OpenAsset(const std::filesystem::path& path)
{
    // pathからassetTypeNameを取得する
    auto it = std::find_if(m_assetPaths.begin(), m_assetPaths.end(),
        [&](const DataAssetPathEntry& entry) { return std::find(entry.paths.begin(), entry.paths.end(), path) != entry.paths.end(); });
    std::string assetTypeName = (it != m_assetPaths.end()) ? it->typeName : "";

    // データアセットを開く
    if (!m_document.Open(path, assetTypeName)) return false;

    SyncPathBuffer();
    return true;
}

#pragma endregion