#include <cstdint>
#include <functional>
#include "Engine/Core/component.h"
#include "Engine/Core/component_pool.h"
#include <cassert>
#include <iostream>
struct CleanupComponent : Component {
    int* count = nullptr;
    Component* peer = nullptr;
    void OnDestroy() override {
        if (peer) assert(peer->GetEnable());
        if (count) ++*count;
        NotifyDestroy(); // Reentrant notification must not run cleanup twice.
    }
};
int main() {
    ComponentPool<CleanupComponent> pool;
    int count = 0;
    auto* first = pool.Create(1); first->count = &count;
    pool.Remove(1); assert(count == 1 && !pool.GetByGameObjectID(1));
    pool.Remove(1); assert(count == 1);
    auto* reused = pool.Create(2); reused->count = &count;
    reused->NotifyDestroy(); reused->NotifyDestroy(); assert(count == 2);
    pool.Remove(2); assert(count == 2);
    auto* a = pool.Create(3); auto* b = pool.Create(4);
    a->count = &count; b->count = &count; a->peer = b; b->peer = a;
    a->NotifyDestroy(); b->NotifyDestroy(); // Scene notification phase, before either removal.
    assert(count == 4);
    pool.Remove(3); pool.Remove(4); assert(count == 4);
    std::cout << "component_cleanup_tests passed\n";
}
