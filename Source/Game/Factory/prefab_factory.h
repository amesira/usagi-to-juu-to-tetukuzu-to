// prefab_factory.h
#pragma once
#include <DirectXMath.h>
using namespace DirectX;

class GameObject;
class IScene;

namespace PrefabFactory
{
    struct PlayerPrefab {
        GameObject* player = nullptr;
    };

    /// @brief PlayerPrefabを生成する関数
    /// @param position 生成位置
    PlayerPrefab CreatePlayerPrefab(IScene* scene, const XMFLOAT3& position);
}
