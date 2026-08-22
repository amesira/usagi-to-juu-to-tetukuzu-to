//---------------------------------------------------
// collision_utility.h [当たり判定ユーティリティ]
// ・内部実装はCollisionShape, OverlapTest, SweepTest, RaycastTestなどに委譲する
// 
// Author：Miu Kitamura
// Date  ：2026/04/29
//---------------------------------------------------
#ifndef COLLISION_UTILITY_H
#define COLLISION_UTILITY_H

#include "collision_types.h"

class TransformComponent;
class BoxColliderComponent;
class SphereColliderComponent;

// 当たり判定処理の窓口
class CollisionUtility {
private:
    friend class CollisionPass;
    friend class CollisionQuery;

    // レイヤーマスクによる当たり判定スキップ
    static bool IsIgnoreLayerPair(int layerA, int layerB);

    //----------------------------------------------------
    // AABB境界情報の計算・判定
    //----------------------------------------------------
    // AABB境界情報の計算
    static Bounds ConvertToBounds(TransformComponent* transform, BoxColliderComponent* collider);
    static Bounds ConvertToBounds(TransformComponent* transform, SphereColliderComponent* collider);

    // AABB同士の衝突判定
    static void CheckAABB(/*out*/ CollisionResult& result, Bounds a, Bounds b);

    //----------------------------------------------------
    // 当たり判定の本格チェック
    // ・コライダーの種類別に当たり判定を行う関数群
    //----------------------------------------------------
    // Box - Box
    static void CheckOBB(
        /*out*/ CollisionResult& result,
        TransformComponent* transformA, BoxColliderComponent* colliderA,
        TransformComponent* transformB, BoxColliderComponent* colliderB);
    // Box - Sphere
    static void CheckOBBSphere(
        /*out*/ CollisionResult& result,
        TransformComponent* transformA, BoxColliderComponent* colliderA,
        TransformComponent* transformB, SphereColliderComponent* colliderB);
    // Sphere - Sphere
    static void CheckSphere(
        /*out*/ CollisionResult& result,
        TransformComponent* transformA, SphereColliderComponent* colliderA,
        TransformComponent* transformB, SphereColliderComponent* colliderB);

    //----------------------------------------------------
    // Rayの判定チェック
    //----------------------------------------------------
    // Ray - Box
    static void CheckRayOBB(
        /*out*/ RaycastHit& hitInfo,
        const DirectX::XMFLOAT3& rayOrigin, const DirectX::XMFLOAT3& rayDirection, float rayLength,
        TransformComponent* transform, BoxColliderComponent* collider);
    // Ray - Sphere
    static void CheckRaySphere(
        /*out*/ RaycastHit& hitInfo,
        const DirectX::XMFLOAT3& rayOrigin, const DirectX::XMFLOAT3& rayDirection, float rayLength,
        TransformComponent* transform, SphereColliderComponent* collider);
    // Ray - AABB
    static void CheckRayAABB(
        /*out*/ RaycastHit& hitInfo,
        const DirectX::XMFLOAT3& rayOrigin, const DirectX::XMFLOAT3& rayDirection, float rayLength,
        Bounds bounds);
};

#endif // COLLISION_UTILITY_H
