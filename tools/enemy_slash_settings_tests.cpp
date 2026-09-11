#include "Game/ActorBehavior/Enemy/E30_Combat/Attack/enemy_attack_settings_asset.h"
#include <cassert>
#include <limits>
#include <iostream>

int main()
{
    using namespace EnemyAttackSettings;
    Data legacy;
    assert(FieldSerialization::DeserializeFields(
        nlohmann::json{{"slashDuration", 0.7f}}, legacy, GetSchema()));
    Sanitize(legacy);
    assert(legacy.slashDamage == 10.0f && !legacy.slashEffectAssetPath.empty());
    Data configured;
    configured.slashBurstTime = 0.2f;
    configured.slashDamage = 18.0f;
    configured.slashEffectAssetPath = "asset/MeshEffect/custom_slash.mesh_effect.json";
    configured.slashEffectPosition = {1, 2, 3};
    configured.slashEffectRotation = {0, 90, 45};
    configured.slashEffectScale = 2.0f;
    configured.slashBoxOffset = {0, 1, 2};
    const auto json = FieldSerialization::SerializeFields(configured, GetSchema());
    Data restored;
    assert(FieldSerialization::DeserializeFields(json, restored, GetSchema()));
    assert(restored.slashEffectAssetPath == configured.slashEffectAssetPath);
    assert(restored.slashEffectPosition.z == 3 && restored.slashEffectRotation.y == 90);
    assert(restored.slashEffectScale == 2 && restored.slashDamage == 18);
    assert(restored.slashBurstTime == configured.slashBurstTime && restored.slashBoxOffset.z == 2);
    restored.slashBurstTime = 100;
    restored.slashDamage = -1;
    restored.slashBoxSize = {-1, 0, std::numeric_limits<float>::infinity()};
    restored.slashEffectPosition.x = std::numeric_limits<float>::quiet_NaN();
    restored.slashKnockbackDuration = 0;
    Sanitize(restored);
    assert(restored.slashBurstTime == restored.slashDuration);
    assert(restored.slashDamage == 0 && restored.slashBoxSize.x > 0 && restored.slashBoxSize.y > 0);
    assert(std::isfinite(restored.slashBoxSize.z) && restored.slashEffectPosition.x == 0);
    assert(restored.slashKnockbackDuration > 0);
    restored.slashDuration = 0;
    Sanitize(restored);
    assert(restored.slashBurstTime == 0);
    std::cout << "Enemy slash settings tests passed\n";
}
