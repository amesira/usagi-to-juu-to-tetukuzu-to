//===================================================
// sweep_test.cpp [移動経路の衝突判定]
//===================================================
#include "sweep_test.h"

#include "collision_shape.h"
#include "overlap_test.h"
#include "Engine/Framework/Component/transform_component.h"
#include "Engine/Framework/Component/collider_component.h"
#include "Utility/mi_math.h"

#include <algorithm>

using namespace DirectX;

/// @brief OBB同士の衝突判定
void SweepTest::CheckOBB(
    CollisionResult& result,
    TransformComponent* tA, BoxColliderComponent* cA,
    TransformComponent* tB, BoxColliderComponent* cB)
{
    result = {};

    // 速度適用前の位置を取得
    const XMFLOAT3 prevPosA = tA->GetPrevPosition();
    const XMFLOAT3 prevPosB = tB->GetPrevPosition();

    // 現在位置を取得
    const XMFLOAT3 currPosA = tA->GetPosition();
    const XMFLOAT3 currPosB = tB->GetPosition();

    // CCBのステップ数を取得
    int ccbStep = (std::max)(1, (std::max)(cA->GetCCBStep(), cB->GetCCBStep()));

    for (int i = 0; i <= ccbStep; i++) {
        // 補間位置を計算
        float t = static_cast<float>(i) / static_cast<float>(ccbStep);

        CollisionBoxShape shapeA = CollisionShape::CreateBox(tA, cA, MiMath::Lerp(prevPosA, currPosA, t));
        CollisionBoxShape shapeB = CollisionShape::CreateBox(tB, cB, MiMath::Lerp(prevPosB, currPosB, t));

        OverlapTest::CheckOBB(result, shapeA, shapeB);
        if (!result.isCollision) continue;

        // TODO: minOverlapではなく、CCB対応のために補正値を加える必要がある
        return;
    }
}

/// @brief OBBとSphereの衝突判定
void SweepTest::CheckOBBSphere(
    CollisionResult& result,
    TransformComponent* tA, BoxColliderComponent* cA,
    TransformComponent* tB, SphereColliderComponent* cB)
{
    result = {};

    // 速度適用前の位置
    const XMFLOAT3 prevPosA = tA->GetPrevPosition();
    const XMFLOAT3 prevPosB = tB->GetPrevPosition();

    // 速度適用後の現在位置
    const XMFLOAT3 currPosA = tA->GetPosition();
    const XMFLOAT3 currPosB = tB->GetPosition();

    // CCBのステップ数を取得
    int ccbStep = (std::max)(1, (std::max)(cA->GetCCBStep(), cB->GetCCBStep()));

    for (int i = 0; i <= ccbStep; i++) {
        // 補間係数
        float t = static_cast<float>(i) / static_cast<float>(ccbStep);

        // === ワールド座標系での中心座標を計算 ===
        CollisionBoxShape box = CollisionShape::CreateBox(tA, cA, MiMath::Lerp(prevPosA, currPosA, t));
        CollisionSphereShape sphere = CollisionShape::CreateSphere(tB, cB, MiMath::Lerp(prevPosB, currPosB, t));

        OverlapTest::CheckOBBSphere(result, box, sphere);
        if (!result.isCollision) continue;

        // === CCB対応 ===
        // 現在判定中の位置と、実際に移動する位置の差を計算
        float diffA = MiMath::Distance(box.center, currPosA);
        float diffB = MiMath::Distance(sphere.center, currPosB);
        float diffMax = diffA > diffB ? diffA : diffB;

        // CCB対応のために補正値を加える
        XMFLOAT3 mtvDirection = MiMath::Normalize(result.mtv);
        float overlap = MiMath::Length(result.mtv);
        result.mtv = MiMath::Multiply(mtvDirection, overlap + diffMax);
        return;
    }
}

// Sphere同士の衝突判定
void SweepTest::CheckSphere(
    CollisionResult& result,
    TransformComponent* tA, SphereColliderComponent* cA,
    TransformComponent* tB, SphereColliderComponent* cB)
{
    result = {};

    // 速度適用前の位置を取得
    const XMFLOAT3 prevPosA = tA->GetPrevPosition();
    const XMFLOAT3 prevPosB = tB->GetPrevPosition();

    // 速度適用後の現在位置を取得
    const XMFLOAT3 currPosA = tA->GetPosition();
    const XMFLOAT3 currPosB = tB->GetPosition();

    // CCBのステップ数を取得
    int ccbStep = (std::max)(1, (std::max)(cA->GetCCBStep(), cB->GetCCBStep()));

    for (int i = 0; i <= ccbStep; i++) {
        // 補間係数
        float t = static_cast<float>(i) / static_cast<float>(ccbStep);

        // ワールド座標系での中心座標、半径を計算
        CollisionSphereShape shapeA = CollisionShape::CreateSphere(tA, cA, MiMath::Lerp(prevPosA, currPosA, t));
        CollisionSphereShape shapeB = CollisionShape::CreateSphere(tB, cB, MiMath::Lerp(prevPosB, currPosB, t));

        OverlapTest::CheckSphere(result, shapeA, shapeB);
        if (result.isCollision) return;
    }
}
