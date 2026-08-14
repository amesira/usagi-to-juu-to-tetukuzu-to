#pragma once
#include <DirectXMath.h>
using namespace DirectX;

class GameObject;
class IScene;

namespace PrefabFactory
{
    /// @brief PlayerPrefab構造体
    struct PlayerPrefab {
        GameObject* player;
        GameObject* runDustParticle;
        GameObject* chargeEffectParticle;
        GameObject* chargeLight;
    };

    /// @brief PlayerPrefabを生成する関数
    /// @param position 生成位置
    PlayerPrefab CreatePlayerPrefab(IScene* scene, const XMFLOAT3& position);
}
