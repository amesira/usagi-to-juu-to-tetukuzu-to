//---------------------------------------------------
// File  ：_/EnemyAI/meta_ai.h
// Date  ：2026/09/09
// Author：Miu Kitamura
// 
// ・シーン内のプレイヤー・敵・戦況情報を共有するAIシステムの管理クラス
//---------------------------------------------------
#ifndef META_AI_H
#define META_AI_H
#include <vector>
#include <DirectXMath.h>
#include "enemy_ai_world_context.h"

// シーン内のプレイヤー・敵・戦況情報を共有する。
class MetaAI {
private:
    struct PlayerInfo {
        EnemyAiWorld::EntityInfo entityInfo;
        const class GameObject* gameObject = nullptr;
        const class TransformComponent* transform = nullptr;
    };
    struct EnemyInfo {
        EnemyAiWorld::EntityInfo entityInfo;
        const class GameObject* gameObject = nullptr;
        const class TransformComponent* transform = nullptr;
    };

    PlayerInfo m_player;
    std::vector<EnemyInfo> m_enemies;
    
public:
    void Initialize(const EnemyAIWorldContext& context);
    void Update(EnemyAIWorldContext& context, float deltaTime);
    void Finalize(const EnemyAIWorldContext& context);

    void RegisterPlayer(class GameObject* playerGameObject);
    void RegisterEnemy(class GameObject* enemyGameObject);
    void UnregisterEnemy(unsigned int id);

    const DirectX::XMFLOAT3& GetPlayerPosition() const { return m_player.entityInfo.position; }
    bool HasPlayer() const { return m_player.transform != nullptr; }

    const std::vector<EnemyInfo>& GetEnemies() const { return m_enemies; }

};

#endif // META_AI_H
