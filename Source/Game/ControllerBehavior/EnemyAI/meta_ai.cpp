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
    // Component破棄後のキャッシュポインタを参照する前に、シーンから再取得する。
    if (m_player.gameObject && context.scene) {
        auto* object = context.scene->GetGameObjectByID(m_player.entityInfo.gameObjectID);
        if (!object || !object->GetActive()) {
            m_player.gameObject = nullptr;
            m_player.transform = nullptr;
        }
        else m_player.transform = object->GetComponent<TransformComponent>();
    }
    if (context.scene) {
        for (auto& enemy : m_enemies) {
            auto* object = context.scene->GetGameObjectByID(enemy.entityInfo.gameObjectID);
            if (!object || !object->GetActive()) {
                enemy.gameObject = nullptr;
                enemy.transform = nullptr;
            }
            else {
                enemy.gameObject = object;
                enemy.transform = object->GetComponent<TransformComponent>();
            }
        }
    }
    if (m_player.transform) {
        m_player.entityInfo.position = m_player.transform->GetPosition();
        m_player.entityInfo.velocity = {};
    }

    for (auto& enemy : m_enemies) {
        if (enemy.transform) {
            enemy.entityInfo.position = enemy.transform->GetPosition();
            enemy.entityInfo.velocity = {};
        }
        else {
            enemy.gameObject = nullptr;
            enemy.transform = nullptr;
            enemy.entityInfo.gameObjectID = -1;
        }
    }

    // 敵リストから無効なエントリを削除する
    std::erase_if(m_enemies, [](const EnemyInfo& enemy) {
        return enemy.gameObject == nullptr || enemy.transform == nullptr;
        });
}

void MetaAI::Finalize(const EnemyAIWorldContext& context)
{
    m_enemies.clear();
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
    UnregisterEnemy(enemyGameObject->GetID());

    EnemyInfo enemyInfo;
    enemyInfo.gameObject = enemyGameObject;
    enemyInfo.transform = enemyGameObject->GetComponent<TransformComponent>();
    enemyInfo.entityInfo.gameObjectID = enemyGameObject->GetID();
    if (enemyInfo.transform) {
        enemyInfo.entityInfo.position = enemyInfo.transform->GetPosition();
        enemyInfo.entityInfo.velocity = {};
    }
    CapsuleColliderComponent* collider = enemyGameObject->GetComponent<CapsuleColliderComponent>();
    if (collider) {
        enemyInfo.entityInfo.radius = collider->GetRadius();
    }

    m_enemies.push_back(enemyInfo);
}

void MetaAI::UnregisterEnemy(unsigned int id)
{
    std::erase_if(m_enemies, [id](const EnemyInfo& enemy) {
        return enemy.entityInfo.gameObjectID == id;
    });
}
