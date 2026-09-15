//---------------------------------------------------
// collision_shape.h [衝突判定用形状]
//---------------------------------------------------
#ifndef COLLISION_SHAPE_H
#define COLLISION_SHAPE_H

#include "collision_types.h"

class TransformComponent;
class BoxColliderComponent;
class SphereColliderComponent;

// OBB判定用形状
struct CollisionBoxShape {
    DirectX::XMFLOAT3 center = {};
    DirectX::XMFLOAT3 scale = { 1.0f, 1.0f, 1.0f };
    DirectX::XMFLOAT4 rotation = { 0.0f, 0.0f, 0.0f, 1.0f };
};

// Sphere判定用形状
struct CollisionSphereShape {
    DirectX::XMFLOAT3 center = {};
    float radius = 0.5f;
};

class CapsuleColliderComponent;
struct CollisionCapsuleShape {
    DirectX::XMFLOAT3 pointA = {};
    DirectX::XMFLOAT3 pointB = {};
    float radius = 0.5f;
};

namespace CollisionShape {
    CollisionCapsuleShape CreateCapsule(TransformComponent* transform,
        CapsuleColliderComponent* collider, const DirectX::XMFLOAT3& transformPosition);
    Bounds ConvertToBounds(TransformComponent* transform, CapsuleColliderComponent* collider);
    // OBB判定用形状の作成
    CollisionBoxShape CreateBox(
        TransformComponent* transform, BoxColliderComponent* collider,
        const DirectX::XMFLOAT3& transformPosition);
    // Sphere判定用形状の作成
    CollisionSphereShape CreateSphere(
        TransformComponent* transform, SphereColliderComponent* collider,
        const DirectX::XMFLOAT3& transformPosition);

    // AABB境界情報の計算
    Bounds ConvertToBounds(TransformComponent* transform, BoxColliderComponent* collider);
    Bounds ConvertToBounds(TransformComponent* transform, SphereColliderComponent* collider);
}

#endif // COLLISION_SHAPE_H
