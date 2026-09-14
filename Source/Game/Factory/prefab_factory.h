// prefab_factory.h
#pragma once
#include <DirectXMath.h>
using namespace DirectX;

class GameObject;
class IScene;
namespace EnemyDefinition { struct Data; }

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

    // === EnemyPrefabを生成する関数 ===
    EnemyPrefab CreateEnemyPrefab(
        IScene* scene,
        const XMFLOAT3& position);
    EnemyPrefab CreateHoverRangedEnemyPrefab(
        IScene* scene,
        const XMFLOAT3& position);
    EnemyPrefab CreateEnemyFromDefinition(IScene* scene, const XMFLOAT3& position, const EnemyDefinition::Data& definition);

    // 純粋なモデルオブジェクトを生成する関数
    GameObject* CreateModelObject(
        IScene* scene,
        const char* modelPath,
        const XMFLOAT3& position,
        const XMFLOAT3& rotation,
        const XMFLOAT3& scaling);
}
