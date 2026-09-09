//---------------------------------------------------
// File  ：_/EnemyAI/navigation_system.h
// Date  ：2026/09/09
// Author：Miu Kitamura
// 
// ・ナビゲーショングリッドの生成・経路探索を担当するクラス
//---------------------------------------------------
#ifndef NAVIGATION_SYSTEM_H
#define NAVIGATION_SYSTEM_H
#include "enemy_ai_world_context.h"

class IScene;

// ナビゲーショングリッドの生成・経路探索を担当する。
class NavigationSystem {
private:

public:
    void Initialize(const EnemyAIWorldContext& context);
    void Finalize(const EnemyAIWorldContext& context);

    // Grid生成・経路探索APIは、セルとPathの型を定義する段階で追加する。
};

#endif // NAVIGATION_SYSTEM_H
