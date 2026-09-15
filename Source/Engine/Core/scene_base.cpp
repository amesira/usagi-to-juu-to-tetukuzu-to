#include "scene_base.h"

#include "Engine/Asset/LevelAsset/level_asset.h"
#include "Engine/engine_service_locator.h"
#include "Game/Factory/level_object_factory.h"

bool SceneBase::LoadLevel(const std::filesystem::path& path)
{
    if (path.empty()) return false;

    LevelAssetLoader* loader = EngineServiceLocator::LevelLoader();
    if (!loader) return false;

    LevelAsset* asset = loader->Get(path);
    if (!asset) return false;

    bool success = false;
    LevelObjectFactory::CreateLevel(this, *asset, &success);
    if (!success) return false;

    m_levelAssetPath = path.lexically_normal();
    m_levelAsset = asset;
    return true;
}
