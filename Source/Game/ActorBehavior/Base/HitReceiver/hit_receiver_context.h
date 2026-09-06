#pragma once
#include <DirectXMath.h>
#include <cstdint>

class GameObject;

namespace HitReceiver
{
    enum class AttackType {
        Slash,
        Shot,
    };

    struct HitStopRequest {
        float duration = 0.0f; // 0なら停止しない
        bool affectAttacker = true;
        bool affectReceiver = true;
    };

    /// @brief 攻撃を受け取った時の結果を表す列挙型
    enum class HitAcceptance {
        Accepted,   // 攻撃を受け入れた

        Rejected,   // 攻撃を拒否した
        Invincible, // 無敵状態
        Guarded,    // ガード状態
        InvalidTarget,  // 攻撃対象が無効
    };

    enum class KnockbackMode {
        RelativeDistance,   // 方向と距離を指定してノックバックする
        TargetPosition,     // 目標位置を指定してノックバックする
    };

    enum class KnockbackMovementMode {
        SetTransformPosition,   // Transformの位置を直接設定する
        SetRigidbodyVelocity,   // Rigidbodyの速度を設定する
    };

    /// @brief ノックバックの移動方法に関する設定
    struct KnockbackMovementSource{
        KnockbackMovementMode mode = KnockbackMovementMode::SetTransformPosition;
        bool useRbGravity = true; // Rigidbodyの重力を使用するかどうか
        float gravity = -9.81f; // 重力加速度
    };;

    struct KnockbackRequest {
        bool enabled = false;

        // ノックバックの開始位置を上書きする設定
        bool overrideStartPosition = false;
        DirectX::XMFLOAT3 startPosition = { 0.0f, 0.0f, 0.0f };

        DirectX::XMFLOAT3 targetPosition = { 0.0f, 0.0f, 0.0f };
        DirectX::XMFLOAT3 direction = { 0.0f, 0.0f, 0.0f };
        float distance = 0.0f;
        float duration = 0.0f;

        KnockbackMode mode = KnockbackMode::RelativeDistance;

        bool overrideMovementSource = false;
        KnockbackMovementSource movementSource;
    };

    struct HitData {
        GameObject* attacker = nullptr;

        float damage = 0.0f;
        DirectX::XMFLOAT3 hitPoint = { 0.0f, 0.0f, 0.0f };
        DirectX::XMFLOAT3 hitDirection = { 0.0f, 0.0f, 0.0f };

        KnockbackRequest knockback;
        AttackType attackType = AttackType::Slash;
        HitStopRequest hitStop;
    };

    struct HitResult {
        HitAcceptance acceptance = HitAcceptance::Rejected;
        float appliedDamage = 0.0f;     // 実際に適用されたダメージ量
        bool startedKnockback = false;  // ノックバックが開始されたかどうか
        bool killed = false;            // 攻撃によって死亡したかどうか

        /// @brief 攻撃が受け入れられたかどうかを判定する
        bool WasAccepted() const { return acceptance == HitAcceptance::Accepted; }
    };
}
