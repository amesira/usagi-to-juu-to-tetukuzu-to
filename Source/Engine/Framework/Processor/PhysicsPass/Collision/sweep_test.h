//---------------------------------------------------
// sweep_test.h [移動経路の衝突判定]
//---------------------------------------------------
#ifndef SWEEP_TEST_H
#define SWEEP_TEST_H

#include "collision_types.h"

class TransformComponent;
class BoxColliderComponent;
class SphereColliderComponent;

namespace SweepTest {
    // Box - Box
    void CheckOBB(
        /*out*/ CollisionResult& result,
        TransformComponent* transformA, BoxColliderComponent* colliderA,
        TransformComponent* transformB, BoxColliderComponent* colliderB);

    // Box - Sphere
    void CheckOBBSphere(
        /*out*/ CollisionResult& result,
        TransformComponent* transformA, BoxColliderComponent* colliderA,
        TransformComponent* transformB, SphereColliderComponent* colliderB);

    // Sphere - Sphere
    void CheckSphere(
        /*out*/ CollisionResult& result,
        TransformComponent* transformA, SphereColliderComponent* colliderA,
        TransformComponent* transformB, SphereColliderComponent* colliderB);
}

#endif // SWEEP_TEST_H
