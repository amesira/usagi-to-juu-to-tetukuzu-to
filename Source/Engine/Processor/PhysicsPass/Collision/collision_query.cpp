//===================================================
// collision_query.h [当たり判定クエリー]
// ・RaycastやSphereCastなど、即時実行型の当たり判定クエリーを提供するクラス。
// 
// Author：Miu Kitamura
// Date  ：2026/04/29
//===================================================
#include "collision_query.h"
#include "collision_utility.h"
#include "collision_shape.h"
#include "overlap_test.h"
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
    if (!scene) return false;

    auto* boxColliderPools = scene->GetComponentPool<BoxColliderComponent>();
    auto* sphereColliderPools = scene->GetComponentPool<SphereColliderComponent>();
    auto* transformPool = scene->GetComponentPool<TransformComponent>();

    if (!transformPool) return false;
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

    if (auto* capsules = scene->GetComponentPool<CapsuleColliderComponent>()) {
        for (auto& capsule : capsules->GetList()) {
            if (!capsule.GetEnable() || !capsule.GetOwner()) continue;
            if (!IsCollisionLayerInMask(capsule.GetLayer(),layerMask)) continue;
            auto* t = transformPool->GetByGameObjectID(capsule.GetOwner()->GetID());
            if (!t || !t->GetEnable()) continue;
            RaycastHit tempHit;
            CollisionUtility::CheckRayCapsule(tempHit,origin,direction,maxDistance,t,&capsule);
            if (tempHit.hit && (!raycastHit.hit || tempHit.hitDistance < closestHitDistance)) {
                tempHit.hitObject = capsule.GetOwner();
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
    if (!scene) return false;

    auto* boxColliderPools = scene->GetComponentPool<BoxColliderComponent>();
    auto* sphereColliderPools = scene->GetComponentPool<SphereColliderComponent>();
    auto* transformPool = scene->GetComponentPool<TransformComponent>();

    if (!transformPool) return false;
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

    if (auto* capsules = scene->GetComponentPool<CapsuleColliderComponent>()) {
        for (auto& capsule : capsules->GetList()) {
            if (!capsule.GetEnable() || !capsule.GetOwner()) continue;
            if (!IsCollisionLayerInMask(capsule.GetLayer(),layerMask)) continue;
            auto* t = transformPool->GetByGameObjectID(capsule.GetOwner()->GetID());
            if (!t || !t->GetEnable()) continue;
            RaycastHit tempHit;
            CapsuleColliderComponent expanded = capsule;
            // Preserve the segment length while expanding the radius.
            expanded.SetRadius(capsule.GetRadius()+radius);
            expanded.SetHeight(capsule.GetHeight()+2.0f*radius);
            CollisionUtility::CheckRayCapsule(tempHit,origin,direction,maxDistance,t,&expanded);
            if (tempHit.hit && (!raycastHit.hit || tempHit.hitDistance < closestHitDistance)) {
                tempHit.hitObject = capsule.GetOwner();
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
    const XMFLOAT3& center, const XMFLOAT3& scale, const XMFLOAT4& rotation, CollisionLayerMask layerMask)
{
    if (!scene) return !outObjects.empty();
    auto* transforms = scene->GetComponentPool<TransformComponent>();
    if (!transforms) return !outObjects.empty();
    const CollisionBoxShape query{center,scale,rotation};
    auto visit = [&](auto* pool, auto test) {
        if (!pool) return;
        for (auto& collider : pool->GetList()) {
            if (!collider.GetEnable() || !collider.GetOwner()) continue;
            if (!IsCollisionLayerInMask(collider.GetLayer(),layerMask)) continue;
            auto* t = transforms->GetByGameObjectID(collider.GetOwner()->GetID());
            if (!t || !t->GetEnable()) continue;
            CollisionResult result;
            test(result,t,&collider);
            if (result.isCollision) outObjects.push_back(&collider);
        }
    };
    visit(scene->GetComponentPool<BoxColliderComponent>(), [&](auto& result, auto* t, auto* c) {
        OverlapTest::CheckOBB(result,query,CollisionShape::CreateBox(t,c,t->GetPosition()));
    });
    visit(scene->GetComponentPool<SphereColliderComponent>(), [&](auto& result, auto* t, auto* c) {
        OverlapTest::CheckOBBSphere(result,query,CollisionShape::CreateSphere(t,c,t->GetPosition()));
    });
    visit(scene->GetComponentPool<CapsuleColliderComponent>(), [&](auto& result, auto* t, auto* c) {
        OverlapTest::CheckCapsuleOBB(result,CollisionShape::CreateCapsule(t,c,t->GetPosition()),query);
    });
    return !outObjects.empty();
}

bool CollisionQuery::OverlapSphere(IScene* scene, std::vector<ColliderComponent*>& outObjects,
    const XMFLOAT3& center, float radius, CollisionLayerMask layerMask)
{
    if (!scene) return !outObjects.empty();
    auto* transforms = scene->GetComponentPool<TransformComponent>();
    if (!transforms) return !outObjects.empty();
    const CollisionSphereShape query{center,radius};
    auto visit = [&](auto* pool, auto test) {
        if (!pool) return;
        for (auto& collider : pool->GetList()) {
            if (!collider.GetEnable() || !collider.GetOwner()) continue;
            if (!IsCollisionLayerInMask(collider.GetLayer(),layerMask)) continue;
            auto* t = transforms->GetByGameObjectID(collider.GetOwner()->GetID());
            if (!t || !t->GetEnable()) continue;
            CollisionResult result;
            test(result,t,&collider);
            if (result.isCollision) outObjects.push_back(&collider);
        }
    };
    visit(scene->GetComponentPool<BoxColliderComponent>(), [&](auto& result, auto* t, auto* c) {
        OverlapTest::CheckOBBSphere(result,CollisionShape::CreateBox(t,c,t->GetPosition()),query);
    });
    visit(scene->GetComponentPool<SphereColliderComponent>(), [&](auto& result, auto* t, auto* c) {
        OverlapTest::CheckSphere(result,query,CollisionShape::CreateSphere(t,c,t->GetPosition()));
    });
    visit(scene->GetComponentPool<CapsuleColliderComponent>(), [&](auto& result, auto* t, auto* c) {
        OverlapTest::CheckCapsuleSphere(result,CollisionShape::CreateCapsule(t,c,t->GetPosition()),query);
    });
    return !outObjects.empty();
}
