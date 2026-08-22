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
    XMFLOAT4 targetRot = MiMath::LookRotation(intent.rotateDirection, { 0.0f, 1.0f, 0.0f });
    if (intent.applyRotateRightNow) {
        context.transform->SetRotation(targetRot);
        return;
    }

    XMFLOAT4 newRot = MiMath::Slerp(context.transform->GetRotation(), targetRot, context.settings().rotationSpeed * deltaTime);
    context.transform->SetRotation(newRot);
}