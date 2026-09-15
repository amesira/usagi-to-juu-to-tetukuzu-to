//===================================================
// raycast_test.cpp [Rayの衝突判定]
//===================================================
#include "raycast_test.h"

#include "Engine/Component/transform_component.h"
#include "Engine/Component/collider_component.h"
#include "Utility/mi_math.h"

#include "collision_shape.h"
#include <algorithm>
#include <cmath>

using namespace DirectX;

#pragma region Rayの判定チェック
// RayとBoxの衝突判定
void RaycastTest::CheckRayOBB(
    RaycastHit& outHitInfo,
    const XMFLOAT3& rayOrigin, const XMFLOAT3& rayDirection, float rayLength,
    TransformComponent* transform, BoxColliderComponent* collider)
{
    outHitInfo = RaycastHit();

    // ワールド座標系での中心座標を計算
    XMFLOAT3 center = MiMath::RotateVector(transform->GetRotation(), collider->GetCenter());
    center = MiMath::Add(center, transform->GetPosition());

    // レイをOBBを無回転とした時のローカル座標系に変換
    XMFLOAT3 localRayOrigin = MiMath::RotateVector(
        XMFLOAT4(
            -transform->GetRotation().x,
            -transform->GetRotation().y,
            -transform->GetRotation().z,
            transform->GetRotation().w
        ),
        MiMath::Subtract(rayOrigin, center)
    );
    XMFLOAT3 localRayDirection = MiMath::RotateVector(
        XMFLOAT4(
            -transform->GetRotation().x,
            -transform->GetRotation().y,
            -transform->GetRotation().z,
            transform->GetRotation().w
        ),
        rayDirection
    );
    localRayDirection = MiMath::Normalize(localRayDirection);

    // AABBとの衝突判定
    Bounds bounds = {
        -collider->GetScale().x * 0.5f, collider->GetScale().x * 0.5f,
        -collider->GetScale().y * 0.5f, collider->GetScale().y * 0.5f,
        -collider->GetScale().z * 0.5f, collider->GetScale().z * 0.5f
    };

    RaycastHit localHitInfo;
    CheckRayAABB(localHitInfo, localRayOrigin, localRayDirection, rayLength, bounds);

    if (localHitInfo.hit) {
        // 衝突している
        outHitInfo.hit = true;
        outHitInfo.hitDistance = localHitInfo.hitDistance;
        outHitInfo.hitPoint = MiMath::Add(rayOrigin, MiMath::Multiply(MiMath::Normalize(rayDirection), localHitInfo.hitDistance));
        outHitInfo.hitNormal = MiMath::RotateVector(transform->GetRotation(), localHitInfo.hitNormal);
    }
    else {
        // 衝突していない
        outHitInfo.hit = false;
    }
}

// RayとSphereの衝突判定
void RaycastTest::CheckRaySphere(
    RaycastHit& outHitInfo,
    const XMFLOAT3& rayOrigin, const XMFLOAT3& rayDirection, const float rayLength,
    TransformComponent* transform, SphereColliderComponent* collider)
{
    outHitInfo = RaycastHit();

    XMFLOAT3 dir = MiMath::Normalize(rayDirection);

    // レイの始点から球の中心へのベクトル
    XMFLOAT3 center = MiMath::RotateVector(transform->GetRotation(), collider->GetCenter());
    center = MiMath::Add(center, transform->GetPosition());
    XMFLOAT3 co = MiMath::Subtract(center, rayOrigin);

    float a = MiMath::Dot(dir, dir);
    float b = MiMath::Dot(co, dir);
    float c = MiMath::Dot(co, co) - MiMath::Pow(collider->GetRadius(), 2);

    if (a == 0.0f) {
        // レイの方向ベクトルがゼロの場合は衝突なし
        outHitInfo.hit = false;
        return;
    }

    float s = b * b - a * c;
    if (s < 0.0f) {
        // 判別式が負の場合は衝突なし
        outHitInfo.hit = false;
        return;
    }

    // 衝突点までの距離を計算
    float sqrtS = sqrtf(s);
    float a1 = (b - sqrtS) / a;
    float a2 = (b + sqrtS) / a;

    // レイの後方で衝突
    if (a1 < 0.0f && a2 < 0.0f) {
        outHitInfo.hit = false;
        return;
    }

    // レイの長さを超える距離で衝突
    if (a1 > rayLength && a2 > rayLength) {
        outHitInfo.hit = false;
        return;
    }

    // 衝突している
    outHitInfo.hit = true;
    float a3 = (a1 >= 0.0f) ? a1 : a2;
    outHitInfo.hitDistance = a3;
    outHitInfo.hitPoint = MiMath::Add(rayOrigin, MiMath::Multiply(dir, a3));
    outHitInfo.hitNormal = MiMath::Normalize(MiMath::Subtract(outHitInfo.hitPoint, center));
}

// RayとAABBの衝突判定
void RaycastTest::CheckRayAABB(
    RaycastHit& outHitInfo,
    const XMFLOAT3& rayOrigin, const XMFLOAT3& rayDirection, float rayLength,
    Bounds bounds)
{
    outHitInfo = RaycastHit{};

    const XMFLOAT3 dir = MiMath::Normalize(rayDirection);
    const float epsilon = 0.000001f;
    float tMin = 0.0f;
    float tMax = rayLength;
    XMFLOAT3 hitNormal = { 0.0f, 0.0f, 0.0f };

    // 各軸のスラブを更新するラムダ関数
    auto updateSlab = [&](float origin, float direction, float minValue, float maxValue, const XMFLOAT3& minNormal, const XMFLOAT3& maxNormal) -> bool
        {
            if (std::fabs(direction) < epsilon) {
                return origin >= minValue && origin <= maxValue;
            }

            float t1 = (minValue - origin) / direction;
            float t2 = (maxValue - origin) / direction;
            XMFLOAT3 nearNormal = minNormal;
            XMFLOAT3 farNormal = maxNormal;

            if (t1 > t2) {
                std::swap(t1, t2);
                std::swap(nearNormal, farNormal);
            }

            if (t1 > tMin) {
                tMin = t1;
                hitNormal = nearNormal;
            }
            tMax = (std::min)(tMax, t2);

            return tMin <= tMax;
        };

    if (!updateSlab(rayOrigin.x, dir.x, bounds.minX, bounds.maxX, { -1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f })) return;
    if (!updateSlab(rayOrigin.y, dir.y, bounds.minY, bounds.maxY, { 0.0f, -1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f })) return;
    if (!updateSlab(rayOrigin.z, dir.z, bounds.minZ, bounds.maxZ, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f, 1.0f })) return;

    const float hitDistance = tMin >= 0.0f ? tMin : tMax;
    if (hitDistance < 0.0f || hitDistance > rayLength) return;

    outHitInfo.hit = true;
    outHitInfo.hitDistance = hitDistance;
    outHitInfo.hitPoint = MiMath::Add(rayOrigin, MiMath::Multiply(dir, hitDistance));
    outHitInfo.hitNormal = hitNormal;
}
#pragma endregion


void RaycastTest::CheckRayCapsule(RaycastHit& hit, const XMFLOAT3& origin,
    const XMFLOAT3& direction, float length, TransformComponent* transform, CapsuleColliderComponent* collider)
{
    hit = {};
    const float dirLength = MiMath::Length(direction);
    if (dirLength <= 1.0e-6f || length < 0.0f) return;
    const auto dir = MiMath::Multiply(direction,1.0f/dirLength);
    const auto capsule = CollisionShape::CreateCapsule(transform,collider,transform->GetPosition());
    const auto axisVector = MiMath::Subtract(capsule.pointB,capsule.pointA);
    const float axisLength = MiMath::Length(axisVector);
    const auto axis = axisLength > 1.0e-6f ? MiMath::Multiply(axisVector,1.0f/axisLength) : XMFLOAT3{0,1,0};
    const auto offset = MiMath::Subtract(origin,capsule.pointA);
    const float axialOrigin = MiMath::Dot(offset,axis), axialDirection = MiMath::Dot(dir,axis);
    auto accept = [&](float t, const XMFLOAT3& center) {
        if (t < 0.0f || t > length || (hit.hit && t >= hit.hitDistance)) return;
        hit.hit = true; hit.hitDistance = t;
        hit.hitPoint = MiMath::Add(origin,MiMath::Multiply(dir,t));
        const auto normal = MiMath::Subtract(hit.hitPoint,center);
        hit.hitNormal = MiMath::Length(normal) > 1.0e-6f ? MiMath::Normalize(normal) : MiMath::Multiply(dir,-1.0f);
    };
    // Cylinder surface, restricted to the interval between cap centers.
    if (axisLength > 1.0e-6f) {
        const auto radialDir = MiMath::Subtract(dir,MiMath::Multiply(axis,axialDirection));
        const auto radialOrigin = MiMath::Subtract(offset,MiMath::Multiply(axis,axialOrigin));
        const float a = MiMath::Dot(radialDir,radialDir), b = MiMath::Dot(radialOrigin,radialDir);
        const float c = MiMath::Dot(radialOrigin,radialOrigin)-capsule.radius*capsule.radius;
        const float discriminant = b*b-a*c;
        if (a > 1.0e-12f && discriminant >= 0.0f) {
            for (float sign : {-1.0f,1.0f}) {
                const float t = (-b+sign*sqrtf(discriminant))/a;
                const float y = axialOrigin+t*axialDirection;
                if (y >= 0.0f && y <= axisLength)
                    accept(t,MiMath::Add(capsule.pointA,MiMath::Multiply(axis,y)));
            }
        }
    }
    // Only accept the exterior hemisphere of each cap, not internal sphere surfaces.
    for (int cap = 0; cap < 2; ++cap) {
        const auto center = cap == 0 ? capsule.pointA : capsule.pointB;
        const auto oc = MiMath::Subtract(origin,center);
        const float b = MiMath::Dot(oc,dir), c = MiMath::Dot(oc,oc)-capsule.radius*capsule.radius;
        const float discriminant = b*b-c;
        if (discriminant < 0.0f) continue;
        for (float sign : {-1.0f,1.0f}) {
            const float t = -b+sign*sqrtf(discriminant);
            const float y = axialOrigin+t*axialDirection;
            if (axisLength <= 1.0e-6f || (cap == 0 ? y <= 0.0f : y >= axisLength)) accept(t,center);
        }
    }
}
