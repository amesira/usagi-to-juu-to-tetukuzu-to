//===================================================
// raycast_test.cpp [Rayの衝突判定]
//===================================================
#include "raycast_test.h"

#include "Engine/Component/transform_component.h"
#include "Engine/Component/collider_component.h"
#include "Utility/mi_math.h"

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
