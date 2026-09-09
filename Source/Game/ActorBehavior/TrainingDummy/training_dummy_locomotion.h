#pragma once
#include <DirectXMath.h>

// 試作用の移動実行。AIから水平速度を受け取り、重力・接地はPhysicsへ任せる。
class TrainingDummyLocomotion {
    class RigidbodyComponent* m_rigidbody = nullptr;
public:
    void Initialize(class GameObject* owner);
    void Move(const DirectX::XMFLOAT3& desiredVelocity);
};
