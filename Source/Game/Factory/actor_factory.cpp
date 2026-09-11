//===================================================
// actor_factory.cpp
// 
// Author：Miu Kitamura
// Date  ：2026/04/14
//===================================================
#include "actor_factory.h"
#include "Engine/Core/game_object.h"
#include "Engine/Core/scene_base.h"

// component
#include "Engine/Component/transform_component.h"
#include "Engine/Component/collider_component.h"
#include "Engine/Component/rigidbody_component.h"
#include "Engine/Component/model_component.h"
#include "Engine/Component/animation_component.h"
#include "Engine/Component/sprite_renderer_component.h"
#include "Engine/Component/sprite_animation_component.h"

// behavior
#include "Game/ActorBehavior/Player/player_behavior.h"
#include "Game/PresBehavior/UI/Player/player_ui_behavior.h"
#include "Prefab/player_prefab_settings_asset.h"

#include "Game/ActorBehavior/Player/P10_Locomotion/player_move_behavior.h"

#include "Game/ActorBehavior/Base/health_behavior.h"
#include "Game/ActorBehavior/Base/HitReceiver/hit_receiver_behavior.h"

#include "Engine/engine_service_locator.h"

// プレイヤー生成
GameObject* ActorFactory::CreatePlayer(
    IScene* scene,
    const XMFLOAT3& position,
    const PlayerPrefabSettings::Data& settings)
{
    GameObject* player = scene->CreateGameObject();
    player->SetName("Player");
    player->SetTag("Player");
    player->SetCollisionLayer(CollisionLayer::Player);
    player->SetRenderLayer(RenderLayer::Player);

    // component生成・登録
    TransformComponent* transform = player->AddComponent<TransformComponent>();
    RigidbodyComponent* rigidbody = player->AddComponent<RigidbodyComponent>();
    CapsuleColliderComponent* collider = player->AddComponent<CapsuleColliderComponent>();

    ModelComponent* model = player->AddComponent<ModelComponent>();
    AnimationComponent* animation = player->AddComponent<AnimationComponent>();

    // component設定
    transform->SetPosition(position);
    transform->SetScaling(settings.scaling);

    rigidbody->SetMass(1.0f);
    rigidbody->SetFriction({ 1.0f, 1.0f, 1.0f });

    collider->SetRadius(settings.colliderRadius);
    collider->SetHeight(settings.colliderHeight);
    collider->SetCenter(settings.colliderCenter);

    ModelResource* modelResource = MODEL_REPOSITORY->GetModel("asset/Model/player_model.fbx");
    model->SetModelResource(modelResource);

    // アニメーションの読み込みはPlayerAnimationControllerで行うため、ここではアニメーションの設定は行わない。

    // behavior生成・登録
    player->AddComponent<HealthBehavior>();
    player->AddComponent<HitReceiverBehavior>();
    player->AddComponent<PlayerBehavior>();
    player->AddComponent<PlayerUiBehavior>();
    return player;
}
