#include "Game/ControllerBehavior/StageBounds/stage_bounds_settings_asset.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <limits>
int main() {
    using namespace StageBounds;
    Settings s; s.min = {-10,0,-10}; s.max = {10,10,10};
    Shape shape{{0,1,0}, {.5f,1,.5f}};
    auto wall = Resolve(s, shape, {100,3,4});
    assert(wall.position.x == 9.5f && wall.position.z == 4);
    DirectX::XMFLOAT3 v{5,2,7}; wall.ClipVelocity(v);
    assert(v.x == 0 && v.y == 2 && v.z == 7);
    v = {-5,2,7}; wall.ClipVelocity(v); assert(v.x == -5);
    auto floor = Resolve(s, shape, {0,-100,0});
    assert(floor.floor && floor.position.y == 0);
    v = {2,-20,3}; floor.ClipVelocity(v); assert(v.y == 0 && v.x == 2);
    v.y = 4; floor.ClipVelocity(v); assert(v.y == 4);
    assert(IsGrounded(s, shape, {0,.05f,0}, .1f));
    assert(!IsGrounded(s, shape, {0,1,0}, .1f));
    auto ceiling = Resolve(s, shape, {0,100,0});
    assert(ceiling.position.y == 8 && !ceiling.floor);
    v = {0,10,0}; ceiling.ClipVelocity(v); assert(v.y == 0);
    auto corner = Resolve(s, shape, {-100,2,100});
    assert(corner.position.x == -9.5f && corner.position.z == 9.5f);
    // The predicted physics step must remain at the floor despite Rigidbody gravity.
    auto output = ConstrainVelocity(s, shape, {0,0,0}, {0,0,0}, {1,1,1}, -9.8f, .1f);
    assert(std::abs((output.y - .98f) * .1f) < .00001f);
    output = ConstrainVelocity(s, shape, {9,3,0}, {100,0,5}, {.5f,1,.5f}, 0, .1f);
    assert(std::abs(9 + output.x * .5f * .1f - 9.5f) < .00001f && output.z == 5);
    s.enableMinY = false;
    assert(!IsGrounded(s, shape, {0,-100,0}, 1));
    assert(Resolve(s, shape, {0,-100,0}).position.y == -100);
    s.enableMaxX = false;
    assert(Resolve(s, shape, {100,2,0}).position.x == 100);
    s.min.x = 20; s.max.x = -20; Sanitize(s); assert(s.min.x == -20 && s.max.x == 20);
    s.min.y = std::numeric_limits<float>::quiet_NaN(); Sanitize(s); assert(s.min.y == -10);
    s.min.x = -1; s.max.x = 1; s.enableMaxX = true;
    auto narrow = Resolve(s, Shape{{}, {2,2,2}}, {100,2,0});
    v = {3,0,0}; narrow.ClipVelocity(v); assert(narrow.position.x == 0 && v.x == 0);
    Settings loaded;
    std::ifstream file("asset/Data/stage_bounds_settings.data.json"); assert(file.good());
    auto json = nlohmann::json::parse(file);
    assert(json["type"] == "StageBoundsSettingsAsset" && FieldSerialization::DeserializeFields(json["data"], loaded, GetSchema()));
    auto saved = FieldSerialization::SerializeFields(loaded, GetSchema());
    assert(!FieldSerialization::DeserializeFields(nlohmann::json{{"enableMinY", "invalid"}}, loaded, GetSchema()));
    assert(FieldSerialization::SerializeFields(loaded, GetSchema()) == saved);
    std::cout << "stage_bounds_tests passed\n";
}
