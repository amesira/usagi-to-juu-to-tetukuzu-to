//===================================================
// File  ：_/Editor/LevelEditor/level_editor_document.cpp
// Date  ：2026/09/02
// Author：Miu Kitamura
//===================================================
#include "level_editor_document.h"

#include "Engine/engine_service_locator.h"

#include <algorithm>
#include <iomanip>
#include <sstream>

namespace
{
    LevelAssetLoader* GetLoader()
    {
        AssetManager* assets = EngineServiceLocator::Asset();
        return assets ? assets->LevelAssetLoader() : nullptr;
    }
}

void LevelEditorDocument::New()
{
    m_asset = LevelAsset{};
    m_asset.SetHeader({ "Level", 1, "New Level" });
    m_assetPath.clear();
    m_dirty = false;
    m_nextObjectNumber = 1;
    m_statusMessage = "New level asset";
}

bool LevelEditorDocument::Load(const std::filesystem::path& path)
{
    LevelAssetLoader* loader = GetLoader();
    LevelAsset loaded;
    if (!loader || path.empty() || !loader->Load(path, loaded))
    {
        m_statusMessage = "Failed to load: " + path.generic_string();
        return false;
    }
    m_asset = std::move(loaded);
    m_assetPath = path.lexically_normal();
    m_dirty = false;
    m_statusMessage = "Loaded: " + m_assetPath.generic_string();
    return true;
}

bool LevelEditorDocument::Save()
{
    if (m_assetPath.empty())
    {
        m_statusMessage = "Enter a path and use Save As";
        return false;
    }
    return SaveAs(m_assetPath);
}

bool LevelEditorDocument::SaveAs(std::filesystem::path path)
{
    if (path.empty()) { // 空のパスは保存できない
        m_statusMessage = "Asset path is empty";
        return false;
    }
    if (path.extension() != ".json" || path.stem().extension() != ".level") {
        path.replace_extension(".level.json");
    }

    std::error_code error;
    if (path.has_parent_path()) std::filesystem::create_directories(path.parent_path(), error);
    if (error)
    {
        m_statusMessage = "Failed to create asset directory";
        return false;
    }

    // ヘッダ情報を更新して保存する
    AssetHeader header = m_asset.GetHeader();
    header.m_type = "Level";
    header.m_formatVersion = 1;
    header.m_name = path.stem().stem().string();
    m_asset.SetHeader(header);
    m_asset.SetFilePath(path);

    LevelAssetLoader* loader = GetLoader();
    if (!loader || !loader->Save(path, m_asset))
    {
        m_statusMessage = "Failed to save: " + path.generic_string();
        return false;
    }
    m_assetPath = path.lexically_normal();
    m_dirty = false;
    m_statusMessage = "Saved: " + m_assetPath.generic_string();
    return true;
}

bool LevelEditorDocument::Reload()
{
    if (m_assetPath.empty())
    {
        m_statusMessage = "No level asset path";
        return false;
    }
    return Load(m_assetPath);
}

#pragma region オブジェクト管理
LevelObjectData* LevelEditorDocument::FindObject(const std::string& id)
{
    auto& objects = m_asset.GetData().objects;
    const auto found = std::find_if(objects.begin(), objects.end(), [&](const auto& object) {
        return object.id == id;
    });
    return found == objects.end() ? nullptr : &*found;
}

LevelObjectData& LevelEditorDocument::AddObject()
{
    LevelObjectData object;
    object.id = GenerateObjectId();
    object.name = "New Level Object";
    m_asset.GetData().objects.push_back(std::move(object));
    m_dirty = true;
    return m_asset.GetData().objects.back();
}

bool LevelEditorDocument::RemoveObject(const std::string& id)
{
    auto& objects = m_asset.GetData().objects;
    const auto found = std::find_if(objects.begin(), objects.end(), [&](const auto& object) {
        return object.id == id;
    });
    if (found == objects.end()) return false;
    objects.erase(found);
    m_dirty = true;
    return true;
}

bool LevelEditorDocument::ScaleAllObjects(float factor, const DirectX::XMFLOAT3& pivot)
{
    if (factor <= 0.0f) return false;

    auto& objects = m_asset.GetData().objects;
    if (objects.empty()) return false;

    for (LevelObjectData& object : objects)
    {
        auto& transform = object.transform;
        transform.position.x = pivot.x + (transform.position.x - pivot.x) * factor;
        transform.position.y = pivot.y + (transform.position.y - pivot.y) * factor;
        transform.position.z = pivot.z + (transform.position.z - pivot.z) * factor;
        transform.scale.x *= factor;
        transform.scale.y *= factor;
        transform.scale.z *= factor;

        // 現在の衝突形状計算はTransformのScaleを使わないため、
        // Colliderのローカル座標と寸法も同じ倍率で拡縮する。
        auto& collider = object.collider;
        collider.center.x *= factor;
        collider.center.y *= factor;
        collider.center.z *= factor;
        collider.boxSize.x *= factor;
        collider.boxSize.y *= factor;
        collider.boxSize.z *= factor;
        collider.sphereRadius *= factor;
    }

    m_dirty = true;
    return true;
}
#pragma endregion

/// @brief レベル内で一意なIDを生成する
std::string LevelEditorDocument::GenerateObjectId()
{
    for (;;)
    {
        std::ostringstream stream;
        stream << "level_object_" << std::setw(4) << std::setfill('0') << m_nextObjectNumber++;
        const std::string id = stream.str();
        if (!FindObject(id)) return id;
    }
}
