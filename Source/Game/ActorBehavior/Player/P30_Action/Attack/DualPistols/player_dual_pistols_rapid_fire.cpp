//===================================================
// File  ：_/DualPistols/player_dual_pistols_rapid_fire.cpp
// Date  ：2026/08/28
// Author：Miu Kitamura
//===================================================
#include "player_dual_pistols_rapid_fire.h"
#include "player_dual_pistols_context.h"

#include "Game/ActorBehavior/Player/player_animation_controller.h"
#include "Game/ActorBehavior/Player/P00_Core/player_context.h"
#include "Game/ActorBehavior/Player/P40_Weapon/player_weapon_controller.h"

#include "Utility/mi_math.h"

void PlayerDualPistolsRapidFire::Initialize(PlayerDualPistolsContext& context)
{

}

void PlayerDualPistolsRapidFire::Start(PlayerDualPistolsContext& context)
{
    if (context.weaponController) {
        context.weaponController->SetWeaponMode(
            PlayerWeaponController::WeaponMode::DualPistols);
    }

    m_isActive = true;
    m_fireTimer = 0.0f;
    m_moveBlendParameter = {};
    m_moveBlendParameterVelocity = {};
    m_aimBlendParameter = 0.0f;
    m_aimBlendParameterVelocity = 0.0f;
    m_fireFlipFlop = 0;

    // 移動リクエストを作成する
    if (context.locomotionController) {
        m_locomotionRequest = PlayerLocomotionController::LocomotionRequest{
            .priority = 10,
            .moveDirSourceInfo = { PlayerLocomotionController::DirectionSource::MoveInput },
            .rotateDirSourceInfo = { PlayerLocomotionController::DirectionSource::CameraForward },
            .speedMultiplier = context.settings().rapidFireMoveSpeedMultiplier,
            .jumpPowerMultiplier = context.settings().rapidFireJumpPowerMultiplier,
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
        m_fireFlipFlop = 1 - m_fireFlipFlop;
        bool fired = false;
        if (m_fireFlipFlop == 0) {
            fired = FireLeftPistol(context);
            if (!fired) {
                Stop(context);
                break;
            }
            context.effects.PlayEffects(context, PlayerDualPistolsEffects::EffectsType::FireLeft);
        }
        else {
            fired = FireRightPistol(context);
            if (!fired) {
                Stop(context);
                break;
            }
            context.effects.PlayEffects(context, PlayerDualPistolsEffects::EffectsType::FireRight);
        }
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
    m_moveBlendParameter = {};
    m_moveBlendParameterVelocity = {};
    m_aimBlendParameter = 0.0f;
    m_aimBlendParameterVelocity = 0.0f;
}

bool PlayerDualPistolsRapidFire::FireVolley(PlayerDualPistolsContext& context)
{
    const bool firedLeft = FireLeftPistol(context);
    const bool firedRight = FireRightPistol(context);
    return firedLeft || firedRight;
}

bool PlayerDualPistolsRapidFire::FireLeftPistol(PlayerDualPistolsContext& context)
{
    const auto& aimResult = context.aim.GetAimResult();
    if (!aimResult.hasLeftMuzzle || !context.weaponController) return false;
    if (!context.weaponController->TryConsume(
            PlayerWeaponController::AttackResourceType::DualPistolsFire)) {
        return false;
    }
    PlayerDualPistolsFiring::FireRequest request;
    request.muzzlePosition = aimResult.leftMuzzlePosition;
    request.fireDirection = aimResult.leftFireDirection;
    request.pistolSide = PlayerDualPistolsFiring::PistolSide::Left;
    context.firing.Fire(context, request);
    return true;
}

bool PlayerDualPistolsRapidFire::FireRightPistol(PlayerDualPistolsContext& context)
{
    const auto& aimResult = context.aim.GetAimResult();
    if (!aimResult.hasRightMuzzle || !context.weaponController) return false;
    if (!context.weaponController->TryConsume(
            PlayerWeaponController::AttackResourceType::DualPistolsFire)) {
        return false;
    }
    PlayerDualPistolsFiring::FireRequest request;
    request.muzzlePosition = aimResult.rightMuzzlePosition;
    request.fireDirection = aimResult.rightFireDirection;
    request.pistolSide = PlayerDualPistolsFiring::PistolSide::Right;
    context.firing.Fire(context, request);
    return true;
}

void PlayerDualPistolsRapidFire::UpdateRapidFireAnimation(PlayerDualPistolsContext& context, float deltaTime)
{
    if (!context.animationController || !context.playerRuntimeState) return;
    if (!context.playerRuntimeState->m_isGrounded) return;

    const auto& settings = context.settings();

    // プレイヤー基準の移動方向をRapidFire用2D BlendTreeへ滑らかに反映する。
    const DirectX::XMFLOAT2& targetMoveParameter =
        context.playerRuntimeState->localMoveParameter;
    m_moveBlendParameter.x = MiMath::SmoothDamp(
        m_moveBlendParameter.x,
        targetMoveParameter.x,
        m_moveBlendParameterVelocity.x,
        0.1f,
        deltaTime);
    m_moveBlendParameter.y = MiMath::SmoothDamp(
        m_moveBlendParameter.y,
        targetMoveParameter.y,
        m_moveBlendParameterVelocity.y,
        0.1f,
        deltaTime);

    // キャラクター共通のカメラ照準方向から上下姿勢を決める。
    const float directionY = context.aim.GetAimResult().cameraRayDirection.y;
    float targetAimBlendParameter = 0.0f;

    if (directionY < settings.rapidFireAimBlendDownStartDirectionY) {
        const float downRange =
            settings.rapidFireAimBlendDownStartDirectionY -
            settings.rapidFireAimBlendDownFullDirectionY;
        if (downRange > 0.0f) {
            targetAimBlendParameter = -MiMath::Clamp(
                (settings.rapidFireAimBlendDownStartDirectionY - directionY) / downRange,
                0.0f,
                1.0f);
        }
    }
    else if (directionY > settings.rapidFireAimBlendUpStartDirectionY) {
        const float upRange =
            settings.rapidFireAimBlendUpFullDirectionY -
            settings.rapidFireAimBlendUpStartDirectionY;
        if (upRange > 0.0f) {
            targetAimBlendParameter = MiMath::Clamp(
                (directionY - settings.rapidFireAimBlendUpStartDirectionY) / upRange,
                0.0f,
                1.0f);
        }
    }

    m_aimBlendParameter = MiMath::SmoothDamp(
        m_aimBlendParameter,
        targetAimBlendParameter,
        m_aimBlendParameterVelocity,
        0.1f,
        deltaTime);

    context.animationController->PlayBlendTree2D(
        PlayerAnimationController::Animation::DualPistolsRapidFire,
        m_moveBlendParameter);
    context.animationController->PlayLayerBlendTree1D(
        PlayerAnimationController::AnimationLayer::DualPistolsRapidFireVertical,
        m_aimBlendParameter);
}
