#include "enemy_melee_attack_slash.h"
#include <algorithm>
#include <cmath>
#include <vector>
#include "Engine/Core/game_object.h"
#include "Engine/Component/transform_component.h"
#include "Engine/Component/collider_component.h"
#include "Engine/Processor/PhysicsPass/Collision/collision_query.h"
#include "Game/ActorBehavior/Enemy/E00_Core/enemy_context.h"
#include "Game/ActorBehavior/Base/HitReceiver/hit_receiver_behavior.h"
#include "Utility/mi_math.h"

#include "Game/ActorBehavior/Enemy/E30_Combat/Attack/enemy_attack_context.h"

void EnemyMeleeAttackSlash::Start(EnemyAttackContext& context)
{
    m_settings = context.settings();
    m_elapsedTime = 0.0f;
    m_hasBursted = false;
    m_active = true;
}

EnemyCombatStatus EnemyMeleeAttackSlash::Update(EnemyAttackContext& context, float deltaTime)
{
    if (!m_active || !context.scene || !context.transform) return EnemyCombatStatus::Failure;
    
    m_elapsedTime += deltaTime;
    if (m_elapsedTime < 0.0f) m_elapsedTime = 0.0f;

    // 攻撃実行
    if (!m_hasBursted && m_elapsedTime >= m_settings.slashBurstTime) {
        m_hasBursted = true;
        Burst(context);
    }

    // 長いフレームでもBurstを先に処理
    if (m_elapsedTime >= m_settings.slashDuration) {
        m_active = false;
        return EnemyCombatStatus::Success;
    }

    return EnemyCombatStatus::Running;
}

void EnemyMeleeAttackSlash::Cancel()
{
    m_active = false;
    m_elapsedTime = 0.0f;
    m_hasBursted = false;
}

void EnemyMeleeAttackSlash::Burst(EnemyAttackContext& context)
{
    const auto rotation = context.transform->GetRotation();
    const auto center = MiMath::Add(context.transform->GetPosition(),
        MiMath::RotateVector(rotation, m_settings.slashBoxOffset));
    const auto direction = MiMath::HorizontalNormalize(context.transform->GetForward());

    // OverlapBoxでプレイヤーのコライダーを取得
    std::vector<ColliderComponent*> colliders;
    CollisionQuery::OverlapBox(context.scene, colliders, center, m_settings.slashBoxSize,
        rotation, CollisionLayerToMask(CollisionLayer::Player));

    for (auto* collider : colliders) {
        auto* target = collider ? collider->GetOwner() : nullptr;
        if (!target || !target->GetActive() || target == context.transform->GetOwner()) continue;
        if (target->GetTag() != "Player") continue;

        auto* receiver = target->GetComponent<HitReceiverBehavior>();
        auto* transform = target->GetComponent<TransformComponent>();
        if (!receiver || !receiver->GetEnable() || !transform) continue;

        HitReceiver::HitData hit;
        hit.attacker = context.transform->GetOwner();
        hit.damage = m_settings.slashDamage;
        hit.hitPoint = transform->GetPosition();
        hit.hitDirection = direction;
        hit.attackType = HitReceiver::AttackType::Slash;
        hit.knockback.enabled = false;
        receiver->ReceiveHit(hit);
        return; // プレイヤーへの攻撃が成功した時点で終了
    }
}
