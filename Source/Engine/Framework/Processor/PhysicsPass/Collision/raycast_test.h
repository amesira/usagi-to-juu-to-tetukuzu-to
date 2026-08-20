//---------------------------------------------------
// raycast_test.h [Rayの衝突判定]
//---------------------------------------------------
#ifndef RAYCAST_TEST_H
#define RAYCAST_TEST_H

#include "collision_types.h"

class TransformComponent;
class BoxColliderComponent;
class SphereColliderComponent;

namespace RaycastTest {
    // Ray - Box
    void CheckRayOBB(
        /*out*/ RaycastHit& outHitInfo,
        const DirectX::XMFLOAT3& rayOrigin, const DirectX::XMFLOAT3& rayDirection, float rayLength,
        TransformComponent* transform, BoxColliderComponent* collider);

    // Ray - Sphere
    void CheckRaySphere(
        /*out*/ RaycastHit& outHitInfo,
        const DirectX::XMFLOAT3& rayOrigin, const DirectX::XMFLOAT3& rayDirection, float rayLength,
        TransformComponent* transform, SphereColliderComponent* collider);

    // Ray - AABB
    void CheckRayAABB(
        /*out*/ RaycastHit& outHitInfo,
        const DirectX::XMFLOAT3& rayOrigin, const DirectX::XMFLOAT3& rayDirection, float rayLength,
        Bounds bounds);
}

#endif // RAYCAST_TEST_H
