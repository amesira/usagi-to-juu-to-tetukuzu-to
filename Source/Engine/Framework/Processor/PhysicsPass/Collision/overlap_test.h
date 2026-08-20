//---------------------------------------------------
// overlap_test.h [形状同士の重なり判定]
//---------------------------------------------------
#ifndef OVERLAP_TEST_H
#define OVERLAP_TEST_H

#include "collision_shape.h"

namespace OverlapTest {
    // AABB同士の衝突判定
    void CheckAABB(/*out*/ CollisionResult& result, Bounds a, Bounds b);

    // OBB同士の衝突判定
    void CheckOBB(
        /*out*/ CollisionResult& result,
        const CollisionBoxShape& shapeA,
        const CollisionBoxShape& shapeB);

    // OBBとSphereの衝突判定
    void CheckOBBSphere(
        /*out*/ CollisionResult& result,
        const CollisionBoxShape& box,
        const CollisionSphereShape& sphere);

    // Sphere同士の衝突判定
    void CheckSphere(
        /*out*/ CollisionResult& result,
        const CollisionSphereShape& shapeA,
        const CollisionSphereShape& shapeB);
}

#endif // OVERLAP_TEST_H
