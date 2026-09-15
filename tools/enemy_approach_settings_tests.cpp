#include "Game/ActorBehavior/Enemy/E30_Combat/Approach/enemy_approach_settings_asset.h"
#include <cassert>
#include <fstream>
#include <iostream>

int main()
{
    using namespace EnemyApproachSettings;
    std::ifstream file("asset/Data/enemy_approach_settings.data.json");
    assert(file);
    nlohmann::json asset;
    file >> asset;
    Data data;
    assert(FieldSerialization::DeserializeFields(asset.at("data"), data, GetSchema()));
    assert(data.facingMode == FacingMode::Target && data.stopDistance == 2.0f);
    data.facingMode = FacingMode::Movement;
    const auto json = FieldSerialization::SerializeFields(data, GetSchema());
    Data restored;
    assert(FieldSerialization::DeserializeFields(json, restored, GetSchema()));
    assert(restored.facingMode == FacingMode::Movement);
    assert(restored.repathInterval == data.repathInterval);
    data.stopDistance = -1;
    data.restartDistanceMargin = -1;
    data.minRepathInterval = 0;
    data.repathInterval = -1;
    data.pathRetryInterval = -1;
    data.maxPathFailures = 0;
    data.stuckCheckInterval = 0;
    data.minProgressDistance = 0;
    Sanitize(data);
    assert(data.stopDistance == 0 && data.restartDistanceMargin == 0);
    assert(data.minRepathInterval > 0 && data.repathInterval >= data.minRepathInterval);
    assert(data.pathRetryInterval >= data.minRepathInterval && data.maxPathFailures >= 1);
    assert(data.stuckCheckInterval > 0 && data.minProgressDistance > 0);
    std::cout << "Enemy approach settings tests passed\n";
}
