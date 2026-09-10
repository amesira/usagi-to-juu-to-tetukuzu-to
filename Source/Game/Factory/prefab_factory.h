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
    struct TrainingDummyPrefab {
        GameObject* dummy = nullptr;
    };
    struct EnemyPrefab {
        GameObject* enemy = nullptr;
    };

    /// @brief PlayerPrefabを生成する関数
    PlayerPrefab CreatePlayerPrefab(
        IScene* scene, 
        const XMFLOAT3& position);

    /// @brief TrainingDummyPrefabを生成する関数
    TrainingDummyPrefab CreateTrainingDummyPrefab(
        IScene* scene,
        const XMFLOAT3& position);

    EnemyPrefab CreateEnemyPrefab(
        IScene* scene,
        const XMFLOAT3& position);
}
