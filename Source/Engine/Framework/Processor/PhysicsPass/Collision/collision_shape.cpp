//===================================================
// collision_shape.cpp [衝突判定用形状]
//===================================================
#include "collision_shape.h"

#include "Engine/Framework/Component/transform_component.h"
#include "Engine/Framework/Component/collider_component.h"
#include "Utility/mi_math.h"

#include <cmath>

using namespace DirectX;

/// @brief BoxColliderの形状を作成する
/// @param transformPosition 外から与えられるTransformの位置
CollisionBoxShape CollisionShape::CreateBox(
    TransformComponent* transform, BoxColliderComponent* collider,
    const XMFLOAT3& transformPosition)
{
    CollisionBoxShape shape;
    shape.center = MiMath::Add(
        MiMath::RotateVector(transform->GetRotation(), collider->GetCenter()),
        transformPosition);
    shape.scale = collider->GetScale();
    shape.rotation = transform->GetRotation();
    return shape;
}

/// @brief SphereColliderの形状を作成する
/// @param transformPosition 外から与えられるTransformの位置
CollisionSphereShape CollisionShape::CreateSphere(
    TransformComponent* transform, SphereColliderComponent* collider,
    const XMFLOAT3& transformPosition)
{
    CollisionSphereShape shape;
    shape.center = MiMath::Add(
        MiMath::RotateVector(transform->GetRotation(), collider->GetCenter()),
        transformPosition);
    shape.radius = collider->GetRadius();
    return shape;
}

#pragma region AABB境界情報の計算

/// @brief BoxColliderのAABB境界情報計算
Bounds CollisionShape::ConvertToBounds(
    TransformComponent* t, BoxColliderComponent* c)
{
    Bounds bounds = {};

    XMMATRIX R = XMMatrixRotationQuaternion(t->GetRotationVector());

    // half extents (ローカル半サイズ)
    float ex = c->GetScale().x * 0.5f;
    float ey = c->GetScale().y * 0.5f;
    float ez = c->GetScale().z * 0.5f;

    // ワールド座標系での中心座標を計算
    XMFLOAT3 pos = MiMath::RotateVector(t->GetRotation(), c->GetCenter());
    pos = MiMath::Add(pos, t->GetPosition());

    // abs(R) * e
    XMFLOAT3 aabbExtents = {
        ex * fabsf(XMVectorGetX(R.r[0])) + ey * fabsf(XMVectorGetX(R.r[1])) + ez * fabsf(XMVectorGetX(R.r[2])),
        ex * fabsf(XMVectorGetY(R.r[0])) + ey * fabsf(XMVectorGetY(R.r[1])) + ez * fabsf(XMVectorGetY(R.r[2])),
        ex * fabsf(XMVectorGetZ(R.r[0])) + ey * fabsf(XMVectorGetZ(R.r[1])) + ez * fabsf(XMVectorGetZ(R.r[2])),
    };

    bounds.minX = pos.x - aabbExtents.x;
    bounds.maxX = pos.x + aabbExtents.x;
    bounds.minY = pos.y - aabbExtents.y;
    bounds.maxY = pos.y + aabbExtents.y;
    bounds.minZ = pos.z - aabbExtents.z;
    bounds.maxZ = pos.z + aabbExtents.z;

    return bounds;
}

/// @brief SphereColliderのAABB境界情報計算
Bounds CollisionShape::ConvertToBounds(
    TransformComponent* t, SphereColliderComponent* c)
{
    Bounds bounds = {};

    // ワールド座標系での中心座標・サイズを計算
    DirectX::XMFLOAT3 pos = {
        t->GetPosition().x + c->GetCenter().x,
        t->GetPosition().y + c->GetCenter().y,
        t->GetPosition().z + c->GetCenter().z
    };
    float radius = c->GetRadius();

    // 頂点の内、最も小さいもの大きいものを計算
    bounds.minX = pos.x - radius;
    bounds.maxX = pos.x + radius;
    bounds.minY = pos.y - radius;
    bounds.maxY = pos.y + radius;
    bounds.minZ = pos.z - radius;
    bounds.maxZ = pos.z + radius;

    return bounds;
}
#pragma endregion
