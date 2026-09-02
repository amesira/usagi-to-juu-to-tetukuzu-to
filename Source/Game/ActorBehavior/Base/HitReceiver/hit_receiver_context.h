#pragma once

#include <DirectXMath.h>
#include <cstdint>

class GameObject;

enum class HitAcceptance {
    Accepted,
    Rejected,
    Invincible,
    Guarded,
    InvalidTarget,
};

enum class KnockbackMode {
    RelativeDistance,
    TargetPosition,
    FollowSourceOffset,
};

enum class KnockbackEasing {
    Linear,
    EaseIn,
    EaseOut,
    EaseInOut,
};

struct KnockbackRequest {
    bool enabled = false;

    DirectX::XMFLOAT3 direction = { 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 targetPosition = { 0.0f, 0.0f, 0.0f };
    float distance = 0.0f;
    float duration = 0.0f;

    KnockbackMode mode = KnockbackMode::RelativeDistance;
    KnockbackEasing easing = KnockbackEasing::EaseOut;
    GameObject* source = nullptr;
};

struct HitData {
    GameObject* attacker = nullptr;
    std::uint32_t attackID = 0;

    float damage = 0.0f;
    DirectX::XMFLOAT3 hitPoint = { 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 hitDirection = { 0.0f, 0.0f, 0.0f };

    KnockbackRequest knockback;
};

struct HitResult {
    HitAcceptance acceptance = HitAcceptance::Rejected;
    float appliedDamage = 0.0f;
    bool startedKnockback = false;
    bool killed = false;

    bool WasAccepted() const { return acceptance == HitAcceptance::Accepted; }
};
