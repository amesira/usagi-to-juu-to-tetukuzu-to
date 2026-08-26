// prefab_factory.h
#pragma once
#include <DirectXMath.h>
using namespace DirectX;

#include "Prefab/player_prefab_factory.h"

class GameObject;
class IScene;

namespace PrefabFactory
{
    /// @brief PlayerPrefabを生成する関数
    /// @param position 生成位置
    inline PlayerPrefabFactory::PlayerPrefab CreatePlayerPrefab(IScene* scene, const XMFLOAT3& position)
    {
        return PlayerPrefabFactory::CreatePlayerPrefab(scene, position);
    }
}
