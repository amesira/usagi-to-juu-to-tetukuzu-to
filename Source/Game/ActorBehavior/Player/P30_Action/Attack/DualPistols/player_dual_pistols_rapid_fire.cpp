//===================================================
// File  ：_/DualPistols/player_dual_pistols_rapid_fire.cpp
// Date  ：2026/08/28
// Author：Miu Kitamura
//===================================================
#include "player_dual_pistols_rapid_fire.h"
#include "player_dual_pistols_context.h"

#include "Game/ActorBehavior/Player/player_animation_controller.h"

void PlayerDualPistolsRapidFire::Initialize(PlayerDualPistolsContext& context)
{

}

void PlayerDualPistolsRapidFire::Start(PlayerDualPistolsContext& context)
{
    m_isActive = true;
    m_fireTimer = 0.0f;

    // 移動リクエストを作成する
    if (context.locomotionController) {
        m_locomotionRequest = PlayerLocomotionController::LocomotionRequest{
            .priority = 10,
            .moveDirSourceInfo = { PlayerLocomotionController::DirectionSource::MoveInput },
            .rotateDirSourceInfo = { PlayerLocomotionController::DirectionSource::CameraForward },
            .canMove = true,
            .canRotate = true,
            .useGravity = true,
            .canJump = true,
            .applyRotateRightNow = true,
        };
        m_locomotionRequestID = context.locomotionController->AddLocomotionRequest(m_locomotionRequest);
    }

    // 最初の発射時にエイムを更新して、正しい方向で発射する
    context.aim.UpdateAim(context, 0.0f);
    FireVolley(context);
}

void PlayerDualPistolsRapidFire::Update(PlayerDualPistolsContext& context, float deltaTime)
{
    if (!m_isActive) return;

    // エイム更新
    context.aim.UpdateAim(context, deltaTime);

    const float fireInterval = context.settings().rapidFireInterval;
    if (fireInterval <= 0.0f) return;

    m_fireTimer += deltaTime;
    while (m_fireTimer >= fireInterval) {
        m_fireTimer -= fireInterval;
        FireVolley(context);
    }

    // アニメーション更新
    UpdateRapidFireAnimation(context, deltaTime);
}

void PlayerDualPistolsRapidFire::Stop(PlayerDualPistolsContext& context)
{
    m_isActive = false;
    m_fireTimer = 0.0f;

    // 移動リクエストをリセットする
    if (context.locomotionController && m_locomotionRequestID != -1) {
        context.locomotionController->RemoveLocomotionRequestByIndex(m_locomotionRequestID);
        m_locomotionRequestID = -1;
    }
}

void PlayerDualPistolsRapidFire::Reset(PlayerDualPistolsContext& context)
{
    m_isActive = false;
    m_fireTimer = 0.0f;
}

void PlayerDualPistolsRapidFire::FireVolley(PlayerDualPistolsContext& context)
{
    const auto& aimResult = context.aim.GetAimResult();

    if (aimResult.hasLeftMuzzle) {
        PlayerDualPistolsFiring::FireRequest request;
        request.muzzlePosition = aimResult.leftMuzzlePosition;
        request.fireDirection = aimResult.leftFireDirection;
        request.pistolSide = PlayerDualPistolsFiring::PistolSide::Left;
        context.firing.Fire(context, request);
    }

    if (aimResult.hasRightMuzzle) {
        PlayerDualPistolsFiring::FireRequest request;
        request.muzzlePosition = aimResult.rightMuzzlePosition;
        request.fireDirection = aimResult.rightFireDirection;
        request.pistolSide = PlayerDualPistolsFiring::PistolSide::Right;
        context.firing.Fire(context, request);
    }
}

void PlayerDualPistolsRapidFire::UpdateRapidFireAnimation(PlayerDualPistolsContext& context, float deltaTime)
{
    context.animationController->PlayAnimation(PlayerAnimationController::Animation::Firing);
}
