//===================================================
// overlap_test.cpp [形状同士の重なり判定]
//===================================================
#include "overlap_test.h"

#include "Utility/mi_math.h"

#include <cfloat>
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


namespace {
    constexpr float CAPSULE_EPSILON = 1.0e-6f;
    XMFLOAT3 ClosestOnSegment(const XMFLOAT3& p, const XMFLOAT3& a, const XMFLOAT3& b) {
        const auto d = MiMath::Subtract(b, a);
        const float lengthSq = MiMath::Dot(d, d);
        const float t = lengthSq > CAPSULE_EPSILON * CAPSULE_EPSILON
            ? MiMath::Clamp(MiMath::Dot(MiMath::Subtract(p, a), d) / lengthSq, 0.0f, 1.0f) : 0.0f;
        return MiMath::Add(a, MiMath::Multiply(d, t));
    }
    XMFLOAT3 Perpendicular(const XMFLOAT3& axis) {
        if (MiMath::Length(axis) < CAPSULE_EPSILON) return {1, 0, 0};
        const auto n = MiMath::Normalize(axis);
        return MiMath::Normalize(MiMath::Cross(n, fabsf(n.x) < 0.8f ? XMFLOAT3{1,0,0} : XMFLOAT3{0,1,0}));
    }
    void CapsuleContact(CollisionResult& result, const XMFLOAT3& a, const XMFLOAT3& b,
        float radius, const XMFLOAT3& fallback) {
        result = {};
        const auto delta = MiMath::Subtract(a, b);
        const float distance = MiMath::Length(delta);
        if (distance > radius) return;
        result.isCollision = true;
        const auto normal = distance > CAPSULE_EPSILON ? MiMath::Multiply(delta, 1.0f / distance) : fallback;
        result.mtv = MiMath::Multiply(normal, radius - distance);
    }
}

void OverlapTest::CheckCapsuleSphere(CollisionResult& result,
    const CollisionCapsuleShape& capsule, const CollisionSphereShape& sphere)
{
    CapsuleContact(result, ClosestOnSegment(sphere.center, capsule.pointA, capsule.pointB), sphere.center,
        capsule.radius + sphere.radius, Perpendicular(MiMath::Subtract(capsule.pointB, capsule.pointA)));
}

void OverlapTest::CheckCapsule(CollisionResult& result,
    const CollisionCapsuleShape& a, const CollisionCapsuleShape& b)
{
    const auto u = MiMath::Subtract(a.pointB, a.pointA);
    const auto v = MiMath::Subtract(b.pointB, b.pointA);
    const auto w = MiMath::Subtract(a.pointA, b.pointA);
    const float aa = MiMath::Dot(u,u), bb = MiMath::Dot(u,v), cc = MiMath::Dot(v,v);
    const float dd = MiMath::Dot(u,w), ee = MiMath::Dot(v,w);
    constexpr float epsSq = CAPSULE_EPSILON * CAPSULE_EPSILON;
    float s = 0.0f, t = 0.0f;
    if (aa <= epsSq) {
        if (cc > epsSq) t = MiMath::Clamp(ee / cc, 0.0f, 1.0f);
    } else if (cc <= epsSq) {
        s = MiMath::Clamp(-dd / aa, 0.0f, 1.0f);
    } else {
        const float denominator = aa * cc - bb * bb;
        if (denominator > epsSq * aa * cc) s = MiMath::Clamp((bb * ee - cc * dd) / denominator, 0.0f, 1.0f);
        t = (bb * s + ee) / cc;
        if (t < 0.0f) { t = 0.0f; s = MiMath::Clamp(-dd / aa, 0.0f, 1.0f); }
        else if (t > 1.0f) { t = 1.0f; s = MiMath::Clamp((bb - dd) / aa, 0.0f, 1.0f); }
    }
    auto fallback = MiMath::Cross(u, v);
    if (MiMath::Length(fallback) > CAPSULE_EPSILON) fallback = MiMath::Normalize(fallback);
    else fallback = Perpendicular(aa > cc ? u : v);
    CapsuleContact(result, MiMath::Add(a.pointA, MiMath::Multiply(u,s)),
        MiMath::Add(b.pointA, MiMath::Multiply(v,t)), a.radius + b.radius, fallback);
}

void OverlapTest::CheckCapsuleOBB(CollisionResult& result,
    const CollisionCapsuleShape& capsule, const CollisionBoxShape& box)
{
    result = {};
    const XMFLOAT4 inverse{-box.rotation.x, -box.rotation.y, -box.rotation.z, box.rotation.w};
    const auto a = MiMath::RotateVector(inverse, MiMath::Subtract(capsule.pointA, box.center));
    const auto b = MiMath::RotateVector(inverse, MiMath::Subtract(capsule.pointB, box.center));
    const auto d = MiMath::Subtract(b, a);
    const float start[3]{a.x,a.y,a.z}, delta[3]{d.x,d.y,d.z};
    const float extent[3]{fabsf(box.scale.x)*0.5f, fabsf(box.scale.y)*0.5f, fabsf(box.scale.z)*0.5f};
    // Squared segment/AABB distance is piecewise quadratic. Split at slab crossings
    // and evaluate the exact minimum in every interval (including its endpoints).
    float cuts[8]{0.0f,1.0f};
    int count = 2;
    for (int axis = 0; axis < 3; ++axis) {
        if (fabsf(delta[axis]) <= CAPSULE_EPSILON) continue;
        for (float sign : {-1.0f, 1.0f}) {
            const float t = (sign * extent[axis] - start[axis]) / delta[axis];
            if (t > 0.0f && t < 1.0f) cuts[count++] = t;
        }
    }
    std::sort(cuts, cuts + count);
    float bestSq = FLT_MAX;
    XMFLOAT3 closestSegment{}, closestBox{};
    auto evaluate = [&](float t) {
        const auto p = MiMath::Add(a, MiMath::Multiply(d,t));
        const XMFLOAT3 q{MiMath::Clamp(p.x,-extent[0],extent[0]),
            MiMath::Clamp(p.y,-extent[1],extent[1]), MiMath::Clamp(p.z,-extent[2],extent[2])};
        const auto diff = MiMath::Subtract(p,q);
        const float distSq = MiMath::Dot(diff,diff);
        if (distSq < bestSq) { bestSq = distSq; closestSegment = p; closestBox = q; }
    };
    for (int i = 0; i + 1 < count; ++i) {
        evaluate(cuts[i]); evaluate(cuts[i+1]);
        const float mid = (cuts[i]+cuts[i+1])*0.5f;
        float quadratic = 0.0f, linear = 0.0f;
        for (int axis = 0; axis < 3; ++axis) {
            const float value = start[axis] + delta[axis]*mid;
            if (value >= -extent[axis] && value <= extent[axis]) continue;
            const float boundary = value < 0.0f ? -extent[axis] : extent[axis];
            quadratic += delta[axis]*delta[axis];
            linear += delta[axis]*(start[axis]-boundary);
        }
        if (quadratic > 0.0f) evaluate(MiMath::Clamp(-linear/quadratic,cuts[i],cuts[i+1]));
    }
    if (bestSq > capsule.radius*capsule.radius) return;
    result.isCollision = true;
    XMFLOAT3 localMtv{};
    if (bestSq > CAPSULE_EPSILON*CAPSULE_EPSILON) {
        const float distance = sqrtf(bestSq);
        localMtv = MiMath::Multiply(MiMath::Subtract(closestSegment,closestBox), (capsule.radius-distance)/distance);
    } else {
        // Deep intersection: conservative separation along the cheapest box face.
        // Include the whole segment, not just the closest point inside the box.
        float best = FLT_MAX;
        for (int axis = 0; axis < 3; ++axis) {
            const float end = start[axis]+delta[axis];
            const float positive = extent[axis]+capsule.radius-(std::min)(start[axis],end);
            const float negative = (std::max)(start[axis],end)+extent[axis]+capsule.radius;
            const float depth = (std::min)(positive,negative);
            if (depth < best) {
                best = depth;
                float values[3]{};
                values[axis] = positive <= negative ? positive : -negative;
                localMtv = {values[0],values[1],values[2]};
            }
        }
    }
    result.mtv = MiMath::RotateVector(box.rotation,localMtv);
}
