//---------------------------------------------------
// File  ：_/E30_Combat/Approach/enemy_approach_locomotion.h
// Date  ：2026/09/10
// Author：Miu Kitamura
//
// ・PathFollowerの方向をLocomotionへ渡す。
//---------------------------------------------------
#pragma once
struct EnemyApproachContext;
class EnemyLocomotionController;

class EnemyApproachLocomotion {
private:
    EnemyLocomotionController* m_controller = nullptr;
    int m_requestHandle = -1;

public:
    bool Update(EnemyApproachContext& context);
    void Cancel();

};
