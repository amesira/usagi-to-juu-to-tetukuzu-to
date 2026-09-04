//===================================================
// File  ：_/Factory/level_object_factory.cpp
// Date  ：2026/09/02
// Author：Miu Kitamura
//===================================================
#include "level_object_factory.h"

#include "Engine/Asset/LevelAsset/level_asset.h"
#include "Engine/Component/collider_component.h"
#include "Engine/Component/model_component.h"
#include "Engine/Component/transform_component.h"
#include "Engine/Core/game_object.h"
#include "Engine/Core/scene_interface.h"
#include "Engine/engine_service_locator.h"

#include <algorithm>

using namespace DirectX;

GameObject* LevelObjectFactory::CreateObject(
    IScene* scene,
    const LevelObjectData& data)
{
    if (!scene) return nullptr;

    ModelResource* modelResource = nullptr;
    if (!data.modelPath.empty())
    {
        auto* repository = EngineServiceLocator::ModelRepository();
        if (!repository) return nullptr;
        modelResource = repository->GetModel(data.modelPath);
        if (!modelResource) return nullptr;
    }

    GameObject* object = scene->CreateGameObject();
    if (!object) return nullptr;
    object->SetName(data.name);

    TransformComponent* transform = object->AddComponent<TransformComponent>();
    if (!transform) return nullptr;
    transform->SetPosition(data.transform.position);
    transform->SetEulerRawAngle({
        XMConvertToRadians(data.transform.rotationDegrees.x),
        XMConvertToRadians(data.transform.rotationDegrees.y),
        XMConvertToRadians(data.transform.rotationDegrees.z),
    });
    transform->SetScaling(data.transform.scale);

    if (data.collider.type == LevelColliderType::Box)
    {
        BoxColliderComponent* collider = object->AddComponent<BoxColliderComponent>();
        if (!collider) return nullptr;
        collider->SetCenter(data.collider.center);
        collider->SetScale(data.collider.boxSize);
    }
    else if (data.collider.type == LevelColliderType::Sphere)
    {
        SphereColliderComponent* collider = object->AddComponent<SphereColliderComponent>();
        if (!collider) return nullptr;
        collider->SetCenter(data.collider.center);
        collider->SetRadius(data.collider.sphereRadius);
    }

    if (modelResource)
    {
        ModelComponent* model = object->AddComponent<ModelComponent>();
        if (!model) return nullptr;
        model->SetModelResource(modelResource);

        auto& slots = model->GetMaterialSlots();
        const size_t materialCount = (std::min)(slots.size(), data.materialNames.size());
        auto* repository = EngineServiceLocator::GetMaterialRepository();
        if (!repository && materialCount > 0) return nullptr;

        for (size_t i = 0; i < materialCount; ++i)
        {
            if (data.materialNames[i].empty()) continue;
            MaterialResource* material = repository->GetMaterial(data.materialNames[i]);
            if (!material) return nullptr;
            slots[i].materialResource = material;
        }
    }

    return object;
}

std::vector<GameObject*> LevelObjectFactory::CreateLevel(
    IScene* scene,
    const LevelData& level,
    bool* success)
{
    std::vector<GameObject*> result;
    result.reserve(level.objects.size());
    bool allCreated = scene != nullptr;

    if (scene)
    {
        for (const LevelObjectData& data : level.objects)
        {
            GameObject* object = CreateObject(scene, data);
            if (!object)
            {
                allCreated = false;
                break;
            }
            result.push_back(object);
        }
    }

    if (success) *success = allCreated;
    return result;
}

std::vector<GameObject*> LevelObjectFactory::CreateLevel(
    IScene* scene,
    const LevelAsset& asset,
    bool* success)
{
    return CreateLevel(scene, asset.GetData(), success);
}
