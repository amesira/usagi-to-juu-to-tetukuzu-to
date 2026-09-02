//===================================================
// File  ：_/Base/HitReceiver/hit_reaction.h
// Date  ：2026/09/02
// Author：Miu Kitamura
//===================================================
#include "hit_reaction.h"

void HitReaction::Initialize(GameObject* owner)
{
    m_owner = owner;
}

void HitReaction::Update(float)
{

}

void HitReaction::OnHit(const HitData&, const HitResult&)
{
    // 点滅

    // ヒットストップ

    // 通知
}

void HitReaction::OnDeath(const HitData&)
{
    // 通知

}

void HitReaction::Reset()
{

}
