#include "Game/ActorBehavior/Enemy/E30_Combat/enemy_combat_tree.h"
#include <cassert>
#include <iostream>

// TreeはContextを参照として渡すだけなので、エンジンに依存せず遷移を検証する。
class EnemyContext {};

struct TestCombat : EnemyCombatBase {
    bool startable = true;
    bool continuable = true;
    bool interruptible = true;
    int starts = 0, updates = 0, finishes = 0, cancels = 0;
    EnemyCombatStatus result = EnemyCombatStatus::Running;
    explicit TestCombat(int priority) : EnemyCombatBase(priority) {}
    bool CanStart(const EnemyContext&) const override { return startable; }
    bool CanContinue(const EnemyContext&) const override { return continuable; }
    bool IsInterruptible(const EnemyContext&) const override { return interruptible; }
    void Start(EnemyContext&) override { ++starts; }
    EnemyCombatStatus Update(EnemyContext&, float) override { ++updates; return result; }
    void Finish(EnemyContext&) override { ++finishes; }
    void Cancel(EnemyContext&) override { ++cancels; }
};

int main()
{
    EnemyContext context;
    TestCombat low(10), equal(10), high(20);
    EnemyCombatTree tree;
    tree.RegisterBehavior(low);
    tree.RegisterBehavior(equal);
    tree.RegisterBehavior(high);
    high.startable = false;
    tree.Update(context, 0.016f);
    assert(tree.GetActiveBehavior() == &low); // 同優先度は登録順
    low.startable = false;
    tree.Update(context, 0.016f);
    assert(low.updates == 2 && low.cancels == 0); // 開始条件と継続条件は独立
    assert(equal.starts == 0); // 同優先度は割り込まない
    high.startable = true;
    low.interruptible = false;
    tree.Update(context, 0.016f);
    assert(high.starts == 0 && low.updates == 3);
    low.interruptible = true;
    tree.Update(context, 0.016f);
    assert(low.cancels == 1 && high.starts == 1 && high.updates == 1);
    high.interruptible = false;
    high.startable = false;
    high.continuable = false;
    tree.Update(context, 0.016f);
    assert(high.cancels == 1 && tree.GetActiveBehavior() == &equal);
    equal.interruptible = false;
    tree.Cancel(context); // Conditionからの強制キャンセル
    tree.Cancel(context);
    assert(equal.cancels == 1 && !tree.GetActiveBehavior());
    for (auto result : {EnemyCombatStatus::Success, EnemyCombatStatus::Failure}) {
        equal.result = result;
        tree.Update(context, 0.016f);
        assert(!tree.GetActiveBehavior());
    }
    assert(equal.finishes == 2 && equal.cancels == 1);
    high.startable = true;
    high.continuable = true;
    tree.Update(context, 0.016f);
    assert(tree.GetActiveBehavior() == &high); // 登録順より数値優先度
    tree.Finalize(context);
    assert(high.cancels == 2 && !tree.GetActiveBehavior());
    tree.Update(context, 0.016f);
    assert(!tree.GetActiveBehavior());
    std::cout << "Enemy combat transition tests passed\n";
}
