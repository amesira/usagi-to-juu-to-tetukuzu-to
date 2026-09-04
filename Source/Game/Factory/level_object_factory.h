#pragma once

#include <vector>

#include "Engine/Asset/LevelAsset/level_data.h"

class GameObject;
class IScene;
class LevelAsset;

namespace LevelObjectFactory
{
    // 失敗時はnullptr。モデル指定がある場合は、生成前にロード可否を検証する。
    GameObject* CreateObject(IScene* scene, const LevelObjectData& data);

    // 作成できたオブジェクトを返す。途中で失敗した場合、successはfalseになる。
    std::vector<GameObject*> CreateLevel(
        IScene* scene,
        const LevelData& level,
        bool* success = nullptr);
    std::vector<GameObject*> CreateLevel(
        IScene* scene,
        const LevelAsset& asset,
        bool* success = nullptr);
}
