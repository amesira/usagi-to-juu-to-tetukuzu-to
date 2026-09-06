//---------------------------------------------------
// bullet_behavior.h
// 
// Author：Miu Kitamura
// Date  ：2026/06/06
//---------------------------------------------------
#ifndef BULLET_BEHAVIOR_H
#define BULLET_BEHAVIOR_H
#include "Engine/Core/game_object_layer.h"
#include "Engine/Component/behavior_component.h"
#include "Engine/Processor/PhysicsPass/Collision/collision_utility.h"

#include <functional>
#include "Game/ActorBehavior/Base/ReactionEffects/hit_stop_sequence_task.h"

class TransformComponent;
class BlinkerBehavior;

class BulletBehavior : public BehaviorComponent {
public:
    // ヒットコールバックの型定義
    using HitCallback = std::function<void(const RaycastHit&)>;

private:
    TransformComponent* m_transform = nullptr;

    // 弾の物理特性
    DirectX::XMFLOAT3 m_currentVelocity = { 0.0f, 0.0f, 0.0f };
    float m_radius = 0.25f;
    float m_lifeTime = 5.0f;
    float m_lifeTimer = 0.0f;
    CollisionLayerMask m_layerMask = COLLISION_LAYER_MASK_ALL;

    bool m_isExpired = false;       // 弾が寿命切れかどうか
    bool m_hasHit = false;          // 何かにヒットしたかどうか
    RaycastHit m_lastHit = {};      // 最後にヒットした情報
    HitCallback m_onHit = nullptr;  // ヒットコールバック関数

    // === 演出関連 ===
    HitStopTask m_hitStopTask;
    BlinkerBehavior* m_blinkerBehavior = nullptr;

public:
    BulletBehavior() = default;
    ~BulletBehavior() = default;

    void Start() override;
    void Update() override;
    void DrawComponentInspector() override;

    // 弾の初期化
    void Initialize(
        const DirectX::XMFLOAT3& velocity,
        float radius,
        float lifeTime,
        CollisionLayerMask layerMask = COLLISION_LAYER_MASK_ALL);

    // 弾の速度の設定・取得
    void SetVelocity(const DirectX::XMFLOAT3& velocity) { m_currentVelocity = velocity; }
    DirectX::XMFLOAT3 GetVelocity() const { return m_currentVelocity; }

    // 弾の半径の設定・取得
    void SetRadius(float radius);
    float GetRadius() const { return m_radius; }

    // 弾の寿命の設定・取得
    void SetLifeTime(float lifeTime) { m_lifeTime = lifeTime; }
    float GetLifeTime() const { return m_lifeTime; }

    // レイヤーマスクの設定・取得
    void SetLayerMask(CollisionLayerMask layerMask) { m_layerMask = layerMask; }
    CollisionLayerMask GetLayerMask() const { return m_layerMask; }

    // ヒットコールバックの設定
    void SetOnHit(HitCallback onHit) { m_onHit = onHit; }

    // 状態の取得
    bool IsExpired() const { return m_isExpired; }
    bool HasHit() const { return m_hasHit; }
    const RaycastHit& GetLastHit() const { return m_lastHit; }

    // 弾の寿命切れ処理
    void Finalize(bool isHitStop = false);
};

#endif // BULLET_BEHAVIOR_H
