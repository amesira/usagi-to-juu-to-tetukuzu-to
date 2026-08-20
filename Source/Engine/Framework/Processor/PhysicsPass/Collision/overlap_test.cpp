//===================================================
// overlap_test.cpp [形状同士の重なり判定]
//===================================================
#include "overlap_test.h"

#include "Utility/mi_math.h"

#include <algorithm>
#include <cmath>

using namespace DirectX;

// AABB同士の衝突判定
void OverlapTest::CheckAABB(CollisionResult& result, Bounds a, Bounds b)
{
    // 衝突判定
    result.isCollision = (
        a.minX <= b.maxX &&
        a.maxX >= b.minX &&
        a.minY <= b.maxY &&
        a.maxY >= b.minY &&
        a.minZ <= b.maxZ &&
        a.maxZ >= b.minZ
        );

    // Aの最小移動ベクトル
    // ・BとAが重なっている場合、AをBの外に出すためにAが移動すべき最小のベクトル
    result.mtv = { 0.0f,0.0f,0.0f };
    if (result.isCollision) {
        float dx = (a.minX < b.minX) ? (b.minX - a.maxX) : (b.maxX - a.minX);
        float dy = (a.minY < b.minY) ? (b.minY - a.maxY) : (b.maxY - a.minY);
        float dz = (a.minZ < b.minZ) ? (b.minZ - a.maxZ) : (b.maxZ - a.minZ);

        // XYZの中で必要な押し出し量が最も小さい軸を採用
        if (abs(dx) < abs(dy) && abs(dx) < abs(dz)) {
            result.mtv.x = dx;
        }
        else if (abs(dy) < abs(dz)) {
            result.mtv.y = dy;
        }
        else {
            result.mtv.z = dz;
        }
    }
}

/// @brief OBB同士の衝突判定
void OverlapTest::CheckOBB(
    CollisionResult& result,
    const CollisionBoxShape& shapeA,
    const CollisionBoxShape& shapeB)
{
    result = {};

    // 中心点間のベクトル
    XMFLOAT3 diff = MiMath::Subtract(shapeB.center, shapeA.center);

    // 分離軸の情報
    XMFLOAT3 ea1 = MiMath::RotateVector(shapeA.rotation, { shapeA.scale.x * 0.5f, 0.0f, 0.0f });
    XMFLOAT3 ea2 = MiMath::RotateVector(shapeA.rotation, { 0.0f, shapeA.scale.y * 0.5f, 0.0f });
    XMFLOAT3 ea3 = MiMath::RotateVector(shapeA.rotation, { 0.0f, 0.0f, shapeA.scale.z * 0.5f });

    XMFLOAT3 eb1 = MiMath::RotateVector(shapeB.rotation, { shapeB.scale.x * 0.5f, 0.0f, 0.0f });
    XMFLOAT3 eb2 = MiMath::RotateVector(shapeB.rotation, { 0.0f, shapeB.scale.y * 0.5f, 0.0f });
    XMFLOAT3 eb3 = MiMath::RotateVector(shapeB.rotation, { 0.0f, 0.0f, shapeB.scale.z * 0.5f });

    XMFLOAT3 L[15] = {
        ea1, ea2, ea3,
        eb1, eb2, eb3,
        MiMath::Cross(ea1, eb1), MiMath::Cross(ea1, eb2), MiMath::Cross(ea1, eb3),
        MiMath::Cross(ea2, eb1), MiMath::Cross(ea2, eb2), MiMath::Cross(ea2, eb3),
        MiMath::Cross(ea3, eb1), MiMath::Cross(ea3, eb2), MiMath::Cross(ea3, eb3)
    };

    // mtv用の保持変数
    float minOverlap = 1000000.0f;
    XMFLOAT3 mtvAxis = { 0.0f,0.0f,0.0f };

    // === 衝突判定処理 ===
    for (int i = 0; i < 15; i++) {
        if (MiMath::Length(L[i]) < 0.001f) continue;

        XMFLOAT3 l = MiMath::Normalize(L[i]);

        // 中心点間の距離を投影
        float interval = abs(MiMath::Dot(diff, l));

        // 半径を投影
        float rA =
            fabsf(MiMath::Dot(ea1, l)) +
            fabsf(MiMath::Dot(ea2, l)) +
            fabsf(MiMath::Dot(ea3, l));
        float rB =
            fabsf(MiMath::Dot(eb1, l)) +
            fabsf(MiMath::Dot(eb2, l)) +
            fabsf(MiMath::Dot(eb3, l));

        // 分離軸が見つかった場合は衝突していない
        if (interval > (rA + rB)) return;

        // 最小移動ベクトルの計算
        if (minOverlap > (rA + rB) - interval) {
            minOverlap = rA + rB - interval;
            mtvAxis = l;
        }
    }

    // === 衝突している場合の処理 ===
    result.isCollision = true;

    // mtvの設定
    if (MiMath::Dot(diff, mtvAxis) > 0.0f) { // 方向を反転
        mtvAxis = MiMath::Multiply(mtvAxis, -1.0f);
    }
    result.mtv = MiMath::Multiply(mtvAxis, minOverlap);
}

/// @brief OBBとSphereの衝突判定
void OverlapTest::CheckOBBSphere(
    CollisionResult& result,
    const CollisionBoxShape& box,
    const CollisionSphereShape& sphere)
{
    result = {};

    // BoxColliderから見たSphereColliderのローカル座標を計算
    // ・BoxColliderをAABBとして扱うため
    XMFLOAT3 localSpherePos = MiMath::RotateVector(
        // 逆回転を適用
        XMFLOAT4(-box.rotation.x, -box.rotation.y, -box.rotation.z, box.rotation.w),
        MiMath::Subtract(sphere.center, box.center));

    // AABBの各軸に沿った最近接点を計算
    XMFLOAT3 halfExtents = MiMath::Multiply(box.scale, 0.5f);
    XMFLOAT3 closestPoint = {
        MiMath::Clamp(localSpherePos.x, -halfExtents.x, halfExtents.x),
        MiMath::Clamp(localSpherePos.y, -halfExtents.y, halfExtents.y),
        MiMath::Clamp(localSpherePos.z, -halfExtents.z, halfExtents.z)
    };

    // 円の方程式による衝突判定
    XMFLOAT3 difference = MiMath::Subtract(closestPoint, localSpherePos);
    float distanceSquared =
        MiMath::Pow(difference.x, 2) +
        MiMath::Pow(difference.y, 2) +
        MiMath::Pow(difference.z, 2);

    // === 衝突判定 ===
    constexpr float CONTACT_EPSILON = 0.0001f;
    if (distanceSquared > sphere.radius * sphere.radius + CONTACT_EPSILON) return;

    result.isCollision = true;

    // 最小移動ベクトルの計算
    float distance = sqrtf(distanceSquared);
    float overlap = sphere.radius - distance;

    // ローカル座標系での最小移動ベクトル
    XMFLOAT3 localMtv = MiMath::Multiply(MiMath::Normalize(difference), overlap);

    // ワールド座標系に変換
    result.mtv = MiMath::RotateVector(box.rotation, localMtv);
}

// Sphere同士の衝突判定
void OverlapTest::CheckSphere(
    CollisionResult& result,
    const CollisionSphereShape& shapeA,
    const CollisionSphereShape& shapeB)
{
    result = {};

    // コライダー間の距離の二乗を計算
    float interval =
        MiMath::Pow(shapeB.center.x - shapeA.center.x, 2) +
        MiMath::Pow(shapeB.center.y - shapeA.center.y, 2) +
        MiMath::Pow(shapeB.center.z - shapeA.center.z, 2);

    // 半径の和の二乗を計算
    float radiusSum = MiMath::Pow(shapeA.radius + shapeB.radius, 2);

    // 衝突判定
    if (interval > radiusSum) return;

    result.isCollision = true;

    // 最小移動ベクトルの計算
    // ・Aが移動すべき最小のベクトルを計算
    float distance = (std::max)(0.0000001f, sqrtf(interval));
    float overlap = distance - (shapeA.radius + shapeB.radius);
    XMFLOAT3 direction = {
        (shapeA.center.x - shapeB.center.x) / distance,
        (shapeA.center.y - shapeB.center.y) / distance,
        (shapeA.center.z - shapeB.center.z) / distance
    };
    result.mtv = {
        -direction.x * overlap,
        -direction.y * overlap,
        -direction.z * overlap
    };
}
