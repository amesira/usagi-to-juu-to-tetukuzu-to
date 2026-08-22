//---------------------------------------------------
// collision_types.h [当たり判定共通型]
//---------------------------------------------------
#ifndef COLLISION_TYPES_H
#define COLLISION_TYPES_H

#include "Engine/Device/direct3d.h"

class GameObject;

// 衝突判定結果
struct CollisionResult {
    bool isCollision = false;
    DirectX::XMFLOAT3 mtv = { 0.0f, 0.0f, 0.0f };
};

// AABB境界情報
struct Bounds {
    float minX = 0.0f, maxX = 0.0f;
    float minY = 0.0f, maxY = 0.0f;
    float minZ = 0.0f, maxZ = 0.0f;
};

// Raycastのヒット情報
struct RaycastHit {
    bool hit = false;
    GameObject* hitObject = nullptr;

    DirectX::XMFLOAT3 hitPoint = { 0.0f, 0.0f, 0.0f };   // ヒットポイント
    DirectX::XMFLOAT3 hitNormal = { 0.0f, 0.0f, 0.0f };  // ヒット法線
    float hitDistance = 0.0f;                            // ヒット距離
};

#endif // COLLISION_TYPES_H
