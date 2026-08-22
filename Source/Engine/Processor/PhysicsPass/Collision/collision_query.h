//---------------------------------------------------
// collision_query.h [当たり判定クエリー]
// ・RaycastやSphereCastなど、即時実行型の当たり判定クエリーを提供するクラス。
// 
// Author：Miu Kitamura
// Date  ：2026/04/29
//---------------------------------------------------
#ifndef COLLISION_QUERY_H
#define COLLISION_QUERY_H

#include "Engine/Device/direct3d.h"
#include "Engine/Core/game_object_layer.h"
#include <vector>
using namespace DirectX;

class GameObject;
class IScene;

class ColliderComponent;

struct RaycastHit;

// 当たり判定クエリークラス
class CollisionQuery {
public:
    // Raycastクエリー
    static bool Raycast(
        IScene* scene,
        /*out*/ RaycastHit& raycastHit,
        const XMFLOAT3& origin, const XMFLOAT3& direction, float maxDistance,
        CollisionLayerMask layerMask = COLLISION_LAYER_MASK_ALL);
    // SphereCastクエリー
    static bool SphereCast(
        IScene* scene,
        /*out*/ RaycastHit& raycastHit,
        const XMFLOAT3& origin, const XMFLOAT3& direction, float radius, float maxDistance,
        CollisionLayerMask layerMask = COLLISION_LAYER_MASK_ALL);

    // OverlapBoxクエリー
    static bool OverlapBox(
        IScene* scene,
        /*out*/ std::vector<ColliderComponent*>& outObjects,
        const XMFLOAT3& center, const XMFLOAT3& scale, const XMFLOAT4& orientation,
        CollisionLayerMask layerMask = COLLISION_LAYER_MASK_ALL);
    // OverlapSphereクエリー
    static bool OverlapSphere(
        IScene* scene,
        /*out*/ std::vector<ColliderComponent*>& outObjects,
        const XMFLOAT3& center, float radius,
        CollisionLayerMask layerMask = COLLISION_LAYER_MASK_ALL);

};

#endif // COLLISION_QUERY_H
