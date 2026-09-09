//===================================================
// File  ：_/EnemyAI/meta_ai.cpp
// Date  ：2026/09/09
// Author：Miu Kitamura
//===================================================
#include "meta_ai.h"

#include "Engine/Core/game_object.h"
#include "Engine/Core/scene_interface.h"

#include "Engine/Component/transform_component.h"

void MetaAI::Initialize(const EnemyAIWorldContext& context)
{
    IScene* scene = context.scene;

    // プレイヤーの取得
    m_player.gameObject = scene->GetGameObjectByName("Player");
    if (m_player.gameObject) {
        m_player.transform = m_player.gameObject->GetComponent<TransformComponent>();
        m_player.entityInfo.gameObjectID = m_player.gameObject->GetID();
    }
}

void MetaAI::Update(EnemyAIWorldContext& context, float deltaTime)
{
    if (m_player.transform) {
        m_player.entityInfo.position = m_player.transform->GetPosition();
        m_player.entityInfo.velocity = {};
    }
}

void MetaAI::Finalize(const EnemyAIWorldContext& context)
{
    m_player.gameObject = nullptr;
    m_player.transform = nullptr;
}