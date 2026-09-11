#include "Game/ControllerBehavior/EnemyAI/attack_coordinator_system.h"
#include <cassert>
#include <iostream>
#include <limits>
#include <unordered_set>

// CoordinatorはContextを直接参照せず、Worldが生存判定を提供する。
struct EnemyAIWorldContext {};

int main()
{
    EnemyAIWorldContext context;
    AttackCoordinatorSystem system;
    std::unordered_set<int> alive{1, 2, 3, 4};
    const auto validate = [&alive](int id) { return alive.contains(id); };
    assert(!system.RequestAttack({1}));
    system.Initialize(context);
    assert(!system.RequestAttack({1})); // Validator未設定
    system.SetEnemyValidator(validate);
    assert(!system.RequestAttack({-1}));
    assert(!system.RequestAttack({99}));
    assert(system.RequestAttack({1}));
    assert(!system.RequestAttack({1})); // 重複して順番を変えない
    assert(system.RequestAttack({2}));
    assert(!system.CanAttack(1));
    system.Update(context, 0.0f);
    assert(system.CanAttack(1) && !system.CanAttack(2));
    assert(system.ConsumeAttackRequest(1));
    assert(!system.CanAttack(1) && !system.ConsumeAttackRequest(1));
    assert(!system.RequestAttack({1}));
    system.Update(context, 100.0f);
    assert(system.IsAttacking(1) && !system.CanAttack(2)); // 攻撃中は時間で解除しない
    assert(system.FinishAttack(1));
    assert(!system.FinishAttack(1)); // 二重解放で間隔を延長しない
    system.Update(context, 0.25f);
    assert(!system.CanAttack(2));
    system.Update(context, 0.25f);
    assert(system.CanAttack(2));
    assert(system.RequestAttack({3}));
    system.Update(context, 2.0f); // 未消費の許可は失効
    assert(!system.HasAttackRequest(2) && !system.CanAttack(3));
    system.Update(context, 0.5f);
    assert(system.CanAttack(3));
    alive.erase(3); // CanAttackの確認と消費の間に無効化
    assert(!system.ConsumeAttackRequest(3));
    assert(!system.HasAttackRequest(3));

    // 複数枠・ID指定の消費・待機中の取り消し
    system.Initialize(context);
    system.SetSettings({2, 0.0f, 1.0f});
    alive.insert(3);
    assert(system.RequestAttack({1}));
    assert(system.RequestAttack({2}));
    assert(system.RequestAttack({3}));
    assert(system.RequestAttack({4}));
    assert(system.CancelAttackRequest(3));
    system.Update(context, 0.0f);
    assert(system.GetAttackPermissions().size() == 2);
    assert(system.ConsumeAttackRequest(2)); // 許可列の先頭でなくても消費可能
    assert(!system.CanAttack(4));
    assert(system.CancelAttackRequest(1));
    system.Update(context, 0.0f);
    assert(system.CanAttack(4));
    alive.erase(2); // 攻撃中の無効化
    alive.erase(4); // 許可済みの無効化
    assert(system.RequestAttack({3}));
    alive.erase(3); // 待機中の無効化
    system.Update(context, 0.0f);
    assert(system.GetWaitingRequests().empty());
    assert(system.GetAttackPermissions().empty());
    assert(system.GetCurrentAttackers().empty());

    // 上限を減らしても既存枠は取り消さない
    alive = {1, 2, 3, 4};
    assert(system.RequestAttack({1}));
    assert(system.RequestAttack({2}));
    assert(system.RequestAttack({3}));
    system.Update(context, 0.0f);
    assert(system.ConsumeAttackRequest(1) && system.ConsumeAttackRequest(2));
    system.SetSettings({1, 0.0f, 1.0f});
    assert(system.FinishAttack(1));
    system.Update(context, 0.0f);
    assert(!system.CanAttack(3));
    assert(system.FinishAttack(2));
    system.Update(context, 0.0f);
    assert(system.CanAttack(3));
    system.Update(context, -100.0f);
    system.Update(context, std::numeric_limits<float>::quiet_NaN());
    assert(system.CanAttack(3));

    // 正の間隔では大きなdtでも同時に複数の新規許可を出さない
    system.Initialize(context);
    system.SetSettings({2, 0.5f, 2.0f});
    assert(system.RequestAttack({1}) && system.RequestAttack({2}));
    system.Update(context, 100.0f);
    assert(system.CanAttack(1) && !system.CanAttack(2));
    system.Update(context, 0.5f);
    assert(system.CanAttack(2));
    system.Finalize(context);
    assert(!system.HasAttackRequest(1));
    assert(!system.RequestAttack({1}));
    assert(system.GetAttackPermissions().empty());
    std::cout << "Attack coordinator tests passed\n";
}
