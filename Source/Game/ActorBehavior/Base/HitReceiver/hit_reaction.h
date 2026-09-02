//---------------------------------------------------
// File  ：_/Base/HitReceiver/hit_reaction.h
// Date  ：2026/09/02
// Author：Miu Kitamura
// 
// ・ヒット時のリアクションの処理をまとめるクラス
//---------------------------------------------------
#pragma once
#include "hit_receiver_context.h"

class GameObject;

class HitReaction {
private:
    GameObject* m_owner = nullptr;

public:
    void Initialize(GameObject* owner);
    void Update(float deltaTime);

    void OnHit(const HitData& hitData, const HitResult& hitResult);
    void OnDeath(const HitData& hitData);
    void Reset();
};
