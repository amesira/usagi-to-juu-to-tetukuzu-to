//---------------------------------------------------
// File  ：_/EnemyAI/meta_ai.h
// Date  ：2026/09/09
// Author：Miu Kitamura
// 
// ・シーン内のプレイヤー・敵・戦況情報を共有するAIシステムの管理クラス
//---------------------------------------------------
#ifndef META_AI_H
#define META_AI_H
#include "enemy_ai_context.h"

// シーン内のプレイヤー・敵・戦況情報を共有する。
class MetaAI {
private:
    
public:
    void Initialize(const EnemyAIContext& context);
    void Update(EnemyAIContext& context, float deltaTime);
    void Finalize(const EnemyAIContext& context);

};

#endif // META_AI_H
