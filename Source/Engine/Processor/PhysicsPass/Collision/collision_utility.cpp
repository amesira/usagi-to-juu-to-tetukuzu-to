//===================================================
// collision_utility.cpp [当たり判定ユーティリティ]
// ・内部実装はCollisionShape, OverlapTest, SweepTest, RaycastTestなどに委譲する
// 
// Author：Miu Kitamura
// Date  ：2026/04/29
//===================================================
#include "collision_utility.h"

#include "collision_shape.h"
#include "overlap_test.h"
#include "sweep_test.h"
#include "raycast_test.h"
#include "Engine/Core/game_object_layer.h"

constexpr bool COLLISION_MATRIX[(int)CollisionLayer::MAX][(int)CollisionLayer::MAX] = {
    //                 Default    Field      Player     Bullet        Enemy
    /*Default   */   { true,      true,      true,      false,       true},
    /*Field     */   { true,      false,     true,      true,        true},
    /*Player    */   { true,      true,      true,      false,       true},
    /*Bullet    */   { false,     false,     false,     true,        true},
    /*Enemy     */   { true,      true,      true,      true,        true},
};

// レイヤーマスクによる当たり判定スキップ
bool CollisionUtility::IsIgnoreLayerPair(int layerA, int layerB)
{
    if (layerA < 0 || layerA >= (int)CollisionLayer::MAX ||
        layerB < 0 || layerB >= (int)CollisionLayer::MAX) {
        return false; // 無効なレイヤー番号の場合はスキップしない
    }

    if (!COLLISION_MATRIX[layerA][layerB]) {
        return true;
    }
    return false;
}

Bounds CollisionUtility::ConvertToBounds(TransformComponent* transform, BoxColliderComponent* collider)
{
    return CollisionShape::ConvertToBounds(transform, collider);
}

Bounds CollisionUtility::ConvertToBounds(TransformComponent* transform, SphereColliderComponent* collider)
{
    return CollisionShape::ConvertToBounds(transform, collider);
}

void CollisionUtility::CheckAABB(CollisionResult& result, Bounds a, Bounds b)
{
    OverlapTest::CheckAABB(result, a, b);
}

void CollisionUtility::CheckOBB(
    CollisionResult& result,
    TransformComponent* transformA, BoxColliderComponent* colliderA,
    TransformComponent* transformB, BoxColliderComponent* colliderB)
{
    SweepTest::CheckOBB(result, transformA, colliderA, transformB, colliderB);
}

void CollisionUtility::CheckOBBSphere(
    CollisionResult& result,
    TransformComponent* transformA, BoxColliderComponent* colliderA,
    TransformComponent* transformB, SphereColliderComponent* colliderB)
{
    SweepTest::CheckOBBSphere(result, transformA, colliderA, transformB, colliderB);
}

void CollisionUtility::CheckSphere(
    CollisionResult& result,
    TransformComponent* transformA, SphereColliderComponent* colliderA,
    TransformComponent* transformB, SphereColliderComponent* colliderB)
{
    SweepTest::CheckSphere(result, transformA, colliderA, transformB, colliderB);
}

void CollisionUtility::CheckRayOBB(
    RaycastHit& hitInfo,
    const DirectX::XMFLOAT3& rayOrigin, const DirectX::XMFLOAT3& rayDirection, float rayLength,
    TransformComponent* transform, BoxColliderComponent* collider)
{
    RaycastTest::CheckRayOBB(hitInfo, rayOrigin, rayDirection, rayLength, transform, collider);
}

void CollisionUtility::CheckRaySphere(
    RaycastHit& hitInfo,
    const DirectX::XMFLOAT3& rayOrigin, const DirectX::XMFLOAT3& rayDirection, float rayLength,
    TransformComponent* transform, SphereColliderComponent* collider)
{
    RaycastTest::CheckRaySphere(hitInfo, rayOrigin, rayDirection, rayLength, transform, collider);
}

void CollisionUtility::CheckRayAABB(
    RaycastHit& hitInfo,
    const DirectX::XMFLOAT3& rayOrigin, const DirectX::XMFLOAT3& rayDirection, float rayLength,
    Bounds bounds)
{
    RaycastTest::CheckRayAABB(hitInfo, rayOrigin, rayDirection, rayLength, bounds);
}
