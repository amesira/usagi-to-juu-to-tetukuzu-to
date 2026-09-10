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

void MetaAI::RegisterPlayer(GameObject* playerGameObject)
{
    if (!playerGameObject) return;

    m_player.gameObject = playerGameObject;
    m_player.transform = playerGameObject->GetComponent<TransformComponent>();
    m_player.entityInfo.gameObjectID = playerGameObject->GetID();
    if (m_player.transform) {
        m_player.entityInfo.position = m_player.transform->GetPosition();
        m_player.entityInfo.velocity = {};
    }
}

void MetaAI::RegisterEnemy(GameObject* enemyGameObject)
{
    if (!enemyGameObject) return;

    EnemyInfo enemyInfo;
    enemyInfo.gameObject = enemyGameObject;
    enemyInfo.transform = enemyGameObject->GetComponent<TransformComponent>();
    enemyInfo.entityInfo.gameObjectID = enemyGameObject->GetID();
    if (enemyInfo.transform) {
        enemyInfo.entityInfo.position = enemyInfo.transform->GetPosition();
        enemyInfo.entityInfo.velocity = {};
    }
    m_enemies.push_back(enemyInfo);
}
