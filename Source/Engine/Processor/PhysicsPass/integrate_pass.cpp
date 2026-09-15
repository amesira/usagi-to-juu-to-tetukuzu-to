//===================================================
// integrate_pass.cpp [物理演算パス]
// 
// ・重力、摩擦などをvelocity（速度）に設定し、位置への適用を行う
// ・物理演算群の1番目にあたる。
// 
// Author：Miu Kitamura
// Date  ：2026/03/10
//===================================================
#include "integrate_pass.h"
#include "Engine/Core/game_object.h"
#include "Engine/Core/scene_interface.h"

#include "Engine/Device/mi_fps.h"

#include "Engine/Component/transform_component.h"
#include "Engine/Component/rigidbody_component.h"

#include "Utility/debug_renderer.h"
#include "Utility/mi_math.h"

void IntegratePass::Initialize()
{
    
}

void IntegratePass::Finalize()
{

}

void IntegratePass::Process(IScene* pScene)
{
    float deltaTime = FPS_GetDeltaTime();
    if (deltaTime <= 0.0f)return;

    auto* rigidbodyPool = pScene->GetComponentPool<RigidbodyComponent>();
    auto* transformPool = pScene->GetComponentPool<TransformComponent>();
    if (rigidbodyPool == nullptr || transformPool == nullptr)return;

    auto& rigidbodyList = rigidbodyPool->GetList();

    for (RigidbodyComponent& r : rigidbodyList) {
        TransformComponent* transform = transformPool->GetByGameObjectID(r.GetOwner()->GetID());
        RigidbodyComponent* rigidbody = &r;

        // コンポーネントが無効ならスキップ
        if (!transform || !rigidbody)continue;
        if (!transform->GetEnable() || !rigidbody->GetEnable())continue;

        // キネマティックなら速度を0にして位置の更新も行わない
        if (rigidbody->GetIsKinematic()) {
            rigidbody->SetVelocity({ 0.0f, 0.0f, 0.0f });
            continue;
        }

        XMFLOAT3 velocity = rigidbody->GetVelocity();
        {
            // 摩擦の適用（空中と地上で摩擦係数を変える）
            const XMFLOAT3 friction = rigidbody->GetIsGrounded() ? rigidbody->GetFriction() : rigidbody->GetAirFriction();
            velocity = MiMath::Multiply(velocity, friction);

            // 重力の適用
            velocity.y += rigidbody->GetGravityScale() * rigidbody->GetMass() * deltaTime;
        }
        rigidbody->SetVelocity(velocity);

        // 位置の更新
        XMFLOAT3 position = transform->GetPosition();
        position = MiMath::Add(position, MiMath::Multiply(velocity, deltaTime));
        transform->SetPosition(position);
    }
}