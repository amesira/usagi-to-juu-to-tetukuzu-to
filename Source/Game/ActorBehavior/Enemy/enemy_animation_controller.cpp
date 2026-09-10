#include "enemy_animation_controller.h"

#include "Game/ActorBehavior/Enemy/P00_Core/enemy_context.h"

void EnemyAnimationController::Initialize(EnemyContext&)
{
    // TODO: AnimationComponentと敵固有のクリップ定義を接続する。
}

void EnemyAnimationController::BeginFrame()
{
}

void EnemyAnimationController::Update(EnemyContext&)
{
    // TODO: Condition、Combat、Locomotionからの要求を解決して再生する。
}

void EnemyAnimationController::Finalize()
{
}
