//===================================================
// collision_query.h [当たり判定クエリー]
// ・RaycastやSphereCastなど、即時実行型の当たり判定クエリーを提供するクラス。
// 
// Author：Miu Kitamura
// Date  ：2026/04/29
//===================================================
#include "collision_query.h"
#include "collision_utility.h"
#include "Engine/Core/game_object.h"
#include "Engine/Core/scene_interface.h"

#include "Engine/Device/mi_fps.h"

#include "Engine/Component/transform_component.h"
#include "Engine/Component/collider_component.h"
#include "Engine/Component/rigidbody_component.h"

#include "Utility/debug_renderer.h"
#include "Utility/mi_math.h"

//===================================================
// RayCastクエリー
//===================================================
// Raycastクエリー
bool CollisionQuery::Raycast(IScene* scene, RaycastHit& raycastHit, 
    const XMFLOAT3& origin, const XMFLOAT3& direction, float maxDistance, CollisionLayerMask layerMask)
{
    raycastHit = RaycastHit{};

    auto* boxColliderPools = scene->GetComponentPool<BoxColliderComponent>();
    auto* sphereColliderPools = scene->GetComponentPool<SphereColliderComponent>();
    auto* transformPool = scene->GetComponentPool<TransformComponent>();

    float closestHitDistance = maxDistance;

    if (boxColliderPools) {
        auto& boxColliders = boxColliderPools->GetList();

        for (BoxColliderComponent& boxCollider : boxColliders) {
            if (!boxCollider.GetEnable()) continue;
            if (!IsCollisionLayerInMask(boxCollider.GetLayer(), layerMask)) continue;
            
            TransformComponent* t = transformPool->GetByGameObjectID(boxCollider.GetOwner()->GetID());
            if (t == nullptr) continue;
            
            // Ray - OBBの当たり判定
            RaycastHit tempHit;
            CollisionUtility::CheckRayOBB(
                /*out*/ tempHit,
                origin, direction, maxDistance,
                t, &boxCollider
            );

            // 最も近いヒットを記録
            if (tempHit.hit && tempHit.hitDistance < closestHitDistance) {
                tempHit.hitObject = boxCollider.GetOwner();
                closestHitDistance = tempHit.hitDistance;
                raycastHit = tempHit;
            }
        }
    }

    if (sphereColliderPools) {
        auto& sphereColliders = sphereColliderPools->GetList();

        for (SphereColliderComponent& sphereCollider : sphereColliders) {
            if (!sphereCollider.GetEnable()) continue;
            if (!IsCollisionLayerInMask(sphereCollider.GetLayer(), layerMask)) continue;
            
            TransformComponent* t = transformPool->GetByGameObjectID(sphereCollider.GetOwner()->GetID());
            if (t == nullptr) continue;
            
            // Ray - Sphereの当たり判定
            RaycastHit tempHit;
            CollisionUtility::CheckRaySphere(
                /*out*/ tempHit,
                origin, direction, maxDistance,
                t, &sphereCollider
            );

            // 最も近いヒットを記録
            if (tempHit.hit && tempHit.hitDistance < closestHitDistance) {
                tempHit.hitObject = sphereCollider.GetOwner();
                closestHitDistance = tempHit.hitDistance;
                raycastHit = tempHit;
            }
        }
    }

    return raycastHit.hit;
}

// SphereCastクエリー
bool CollisionQuery::SphereCast(IScene* scene, RaycastHit& raycastHit, 
    const XMFLOAT3& origin, const XMFLOAT3& direction, float radius, float maxDistance, CollisionLayerMask layerMask)
{
    raycastHit = RaycastHit{};

    auto* boxColliderPools = scene->GetComponentPool<BoxColliderComponent>();
    auto* sphereColliderPools = scene->GetComponentPool<SphereColliderComponent>();
    auto* transformPool = scene->GetComponentPool<TransformComponent>();

    float closestHitDistance = maxDistance;

    if (boxColliderPools) {
        auto& boxColliders = boxColliderPools->GetList();

        for (BoxColliderComponent& boxCollider : boxColliders) {
            if (!boxCollider.GetEnable()) continue;
            if (!IsCollisionLayerInMask(boxCollider.GetLayer(), layerMask)) continue;

            TransformComponent* t = transformPool->GetByGameObjectID(boxCollider.GetOwner()->GetID());
            if (t == nullptr) continue;

            // SphereCast用の一時的なBoxColliderを作成
            BoxColliderComponent tempBoxCollider = boxCollider;
            tempBoxCollider.SetScale(XMFLOAT3(
                boxCollider.GetScale().x + radius * 2.0f,
                boxCollider.GetScale().y + radius * 2.0f,
                boxCollider.GetScale().z + radius * 2.0f
            ));

            // Ray - OBBの当たり判定
            RaycastHit tempHit;
            CollisionUtility::CheckRayOBB(
                /*out*/ tempHit,
                origin, direction, maxDistance,
                t, &tempBoxCollider
            );

            // 最も近いヒットを記録
            if (tempHit.hit && tempHit.hitDistance < closestHitDistance) {
                tempHit.hitObject = boxCollider.GetOwner();
                closestHitDistance = tempHit.hitDistance;
                raycastHit = tempHit;
            }
        }
    }

    if (sphereColliderPools) {
        auto& sphereColliders = sphereColliderPools->GetList();

        for (SphereColliderComponent& sphereCollider : sphereColliders) {
            if (!sphereCollider.GetEnable()) continue;
            if (!IsCollisionLayerInMask(sphereCollider.GetLayer(), layerMask)) continue;

            TransformComponent* t = transformPool->GetByGameObjectID(sphereCollider.GetOwner()->GetID());
            if (t == nullptr) continue;

            // SphereCast用の一時的なSphereColliderを作成
            SphereColliderComponent tempSphereCollider = sphereCollider;
            tempSphereCollider.SetRadius(sphereCollider.GetRadius() + radius);

            // Ray - Sphereの当たり判定
            RaycastHit tempHit;
            CollisionUtility::CheckRaySphere(
                /*out*/ tempHit,
                origin, direction, maxDistance,
                t, &tempSphereCollider
            );

            // 最も近いヒットを記録
            if (tempHit.hit && tempHit.hitDistance < closestHitDistance) {
                tempHit.hitObject = sphereCollider.GetOwner();
                closestHitDistance = tempHit.hitDistance;
                raycastHit = tempHit;
            }
        }
    }

    return raycastHit.hit;
}

//===================================================
// Overlapクエリ―
//===================================================
// OverlapBoxクエリー
bool CollisionQuery::OverlapBox(IScene* scene, std::vector<ColliderComponent*>& outObjects, 
    const XMFLOAT3& center, const XMFLOAT3& scale, const XMFLOAT4& orientation,
    CollisionLayerMask layerMask)
{
    auto* boxColliderPools = scene->GetComponentPool<BoxColliderComponent>();
    auto* sphereColliderPools = scene->GetComponentPool<SphereColliderComponent>();
    auto* transformPool = scene->GetComponentPool<TransformComponent>();

    // 判定用の一時Componentを作成
    TransformComponent tempTransform;
    BoxColliderComponent tempBoxCollider;
    {
        tempTransform.SetPosition(center);
        tempTransform.SetRotation(orientation);
        tempBoxCollider.SetScale(scale);
    }

    if (boxColliderPools) {
        auto& boxColliders = boxColliderPools->GetList();

        for (BoxColliderComponent& boxCollider : boxColliders) {
            if (!boxCollider.GetEnable()) continue;
            if (!IsCollisionLayerInMask(boxCollider.GetLayer(), layerMask)) continue;
            
            TransformComponent* t = transformPool->GetByGameObjectID(boxCollider.GetOwner()->GetID());
            
            if (t == nullptr) continue;
            
            CollisionResult result;
            CollisionUtility::CheckOBB(
                /*out*/ result,
                t, &boxCollider,
                &tempTransform, &tempBoxCollider
            );

            if (result.isCollision) {
                outObjects.push_back(&boxCollider);
            }
        }
    }

    if (sphereColliderPools) {
        auto& sphereColliders = sphereColliderPools->GetList();
    
        for (SphereColliderComponent& sphereCollider : sphereColliders) {
            if (!sphereCollider.GetEnable()) continue;
            if (!IsCollisionLayerInMask(sphereCollider.GetLayer(), layerMask)) continue;
    
            TransformComponent* t = transformPool->GetByGameObjectID(sphereCollider.GetOwner()->GetID());
    
            if (t == nullptr) continue;
    
            CollisionResult result;
            CollisionUtility::CheckOBBSphere(
                /*out*/ result,
                &tempTransform, &tempBoxCollider,
                t, &sphereCollider
            );
    
            if (result.isCollision) {
                outObjects.push_back(&sphereCollider);
            }
        }
    }

    return !outObjects.empty();
}

// OverlapSphereクエリー
bool CollisionQuery::OverlapSphere(IScene* scene, std::vector<ColliderComponent*>& outObjects, 
    const XMFLOAT3& center, float radius, CollisionLayerMask layerMask)
{
    auto* boxColliderPools = scene->GetComponentPool<BoxColliderComponent>();
    auto* sphereColliderPools = scene->GetComponentPool<SphereColliderComponent>();
    auto* transformPool = scene->GetComponentPool<TransformComponent>();

    // 判定用の一時Componentを作成
    TransformComponent tempTransform;
    SphereColliderComponent tempSphereCollider;
    {
        tempTransform.SetPosition(center);
        tempSphereCollider.SetRadius(radius);
    }

    if (boxColliderPools) {
        auto& boxColliders = boxColliderPools->GetList();
        for (BoxColliderComponent& boxCollider : boxColliders) {
            if (!boxCollider.GetEnable()) continue;
            if (!IsCollisionLayerInMask(boxCollider.GetLayer(), layerMask)) continue;
            
            TransformComponent* t = transformPool->GetByGameObjectID(boxCollider.GetOwner()->GetID());
            
            if (t == nullptr) continue;
            
            CollisionResult result;
            CollisionUtility::CheckOBBSphere(
                /*out*/ result,
                t, &boxCollider,
                &tempTransform, &tempSphereCollider
            );

            if (result.isCollision) {
                outObjects.push_back(&boxCollider);
            }
        }
    }

    if (sphereColliderPools) {
        auto& sphereColliders = sphereColliderPools->GetList();
        for (SphereColliderComponent& sphereCollider : sphereColliders) {
            if (!sphereCollider.GetEnable()) continue;
            if (!IsCollisionLayerInMask(sphereCollider.GetLayer(), layerMask)) continue;
            
            TransformComponent* t = transformPool->GetByGameObjectID(sphereCollider.GetOwner()->GetID());
            
            if (t == nullptr) continue;
            
            CollisionResult result;
            CollisionUtility::CheckSphere(
                /*out*/ result,
                t, &sphereCollider,
                &tempTransform, &tempSphereCollider
            );

            if (result.isCollision) {
                outObjects.push_back(&sphereCollider);
            }
        }
    }

    return !outObjects.empty();
}
