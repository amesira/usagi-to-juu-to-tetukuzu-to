#include "Game/ActorBehavior/Enemy/E30_Combat/Attack/enemy_attack_settings_asset.h"
#include <cassert>
#include <limits>
#include <iostream>

int main()
{
    using namespace EnemyAttackSettings;
    Data legacy;
    assert(FieldSerialization::DeserializeFields(nlohmann::json{{"shotCount", 3}}, legacy, GetSchema()));
    assert(legacy.firstShotDelay == 0.1f && legacy.shotPlaybackSpeed == 1.0f);
    Data configured;
    configured.firstShotDelay = 0.15f;
    configured.shotPlaybackSpeed = 2.0f;
    Data restored;
    assert(FieldSerialization::DeserializeFields(FieldSerialization::SerializeFields(configured, GetSchema()), restored, GetSchema()));
    assert(restored.firstShotDelay == configured.firstShotDelay && restored.shotPlaybackSpeed == 2.0f);
    restored.firstShotDelay = 100.0f;
    restored.shotPlaybackSpeed = 0.0f;
    Sanitize(restored);
    assert(restored.firstShotDelay == restored.shotInterval && restored.shotPlaybackSpeed > 0);
    restored.firstShotDelay = std::numeric_limits<float>::quiet_NaN();
    Sanitize(restored);
    assert(std::isfinite(restored.firstShotDelay));
    std::cout << "Enemy shot settings tests passed\n";
}
