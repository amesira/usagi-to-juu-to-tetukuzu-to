//===================================================
// File  ：_/Player/Movement/player_move_rotate.cpp
// Date  ：2026/08/15
// Author：Miu Kitamura
// 
// ・PlayerMoveBehaviorの回転処理を担当するクラス
//===================================================
#include "player_move_rotate.h"

#include "Utility/mi_math.h"
using namespace DirectX;

#include "Game/ActorBehavior/Player/P10_Locomotion/player_move_context.h"
#include "Game/ActorBehavior/Player/P10_Locomotion/player_move_intent.h"

// component
#include "Engine/Component/transform_component.h"

/// @brief PlayerMoveの回転処理を更新する
void PlayerMoveRotate::UpdateRotate(PlayerMoveContext& context, const PlayerMoveIntent& intent, float deltaTime)
{
    XMFLOAT4 targetRot = {};

    // 強制回転
    if (intent.forceRotateIntent.isActive) {
        targetRot = MiMath::LookRotation(
            intent.forceRotateIntent.targetDirection, { 0.0f, 1.0f, 0.0f });
    }
    // 通常回転
    else if (intent.canRotate && MiMath::Length(intent.rotateDirection) > 0.01f) {
        targetRot = MiMath::LookRotation(
            intent.rotateDirection, { 0.0f, 1.0f, 0.0f });

        if (!intent.applyRotateRightNow) {
            targetRot = MiMath::Slerp(
                context.transform->GetRotation(), targetRot,
                context.settings().rotationSpeed * deltaTime);
        }
    }
    else {
        return;
    }

    context.transform->SetRotation(targetRot);
}