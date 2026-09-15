// Standalone regression tests; run with run_capsule_tests.py from the repository root.
#include "Engine/Component/collider_component.h"
#include "Engine/Component/transform_component.h"
#include "Engine/Processor/PhysicsPass/Collision/collision_shape.h"
#include "Engine/Processor/PhysicsPass/Collision/overlap_test.h"
#include "Engine/Processor/PhysicsPass/Collision/raycast_test.h"
#include "Engine/Processor/PhysicsPass/Collision/sweep_test.h"
#include <cstdlib>
#include <iostream>
#include <random>

namespace {
int checks = 0;
void Require(bool condition, const char* message) {
    ++checks;
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; std::exit(1); }
}
bool Near(float a, float b) { return fabsf(a-b) < 0.0002f; }
CollisionCapsuleShape Move(CollisionCapsuleShape s, XMFLOAT3 offset) {
    s.pointA = MiMath::Add(s.pointA,offset); s.pointB = MiMath::Add(s.pointB,offset); return s;
}
void CheckSeparated(CollisionCapsuleShape capsule, const CollisionBoxShape& box) {
    CollisionResult hit;
    OverlapTest::CheckCapsuleOBB(hit,capsule,box);
    if (!hit.isCollision) return;
    Require(std::isfinite(hit.mtv.x) && std::isfinite(hit.mtv.y) && std::isfinite(hit.mtv.z), "finite box MTV");
    if (MiMath::Length(hit.mtv) < 0.00001f) return;
    const auto movement = MiMath::Add(hit.mtv,MiMath::Multiply(MiMath::Normalize(hit.mtv),0.001f));
    OverlapTest::CheckCapsuleOBB(hit,Move(capsule,movement),box);
    Require(!hit.isCollision, "MTV separates complete capsule from box");
}
}

int main() {
    CapsuleColliderComponent component;
    component.SetRadius(0.5f); component.SetHeight(4.0f);
    TransformComponent transform;
    transform.SetPosition({3,4,5}); transform.SetEulerAngle({0,0,XM_PIDIV2});
    component.SetCenter({0,1,0});
    auto shape = CollisionShape::CreateCapsule(&transform,&component,transform.GetPosition());
    Require(Near(shape.pointA.x,3.5f) && Near(shape.pointB.x,0.5f), "rotated center and axis");
    auto bounds = CollisionShape::ConvertToBounds(&transform,&component);
    Require(Near(bounds.minX,0) && Near(bounds.maxX,4) && Near(bounds.minY,3.5f), "capsule world bounds");
    component.SetHeight(0); Require(Near(component.GetHeight(),1), "height clamp");
    component.SetRadius(2); Require(Near(component.GetHeight(),4), "radius preserves valid height");
    component.SetRadius(-1); Require(Near(component.GetRadius(),0), "negative radius clamp");

    const CollisionCapsuleShape capsule{{0,-1,0},{0,1,0},0.5f};
    CollisionResult hit;
    OverlapTest::CheckCapsuleSphere(hit,capsule,{{0.8f,0,0},0.5f});
    Require(hit.isCollision && Near(hit.mtv.x,-0.2f), "sphere side and MTV direction");
    OverlapTest::CheckCapsuleSphere(hit,capsule,{{0,1.8f,0},0.5f});
    Require(hit.isCollision && Near(hit.mtv.y,-0.2f), "sphere cap");
    OverlapTest::CheckCapsuleSphere(hit,capsule,{{0,2.1f,0},0.5f});
    Require(!hit.isCollision, "separated sphere");
    OverlapTest::CheckCapsuleSphere(hit,capsule,{{0,0,0},0.5f});
    Require(hit.isCollision && Near(MiMath::Length(hit.mtv),1), "sphere center on axis");
    OverlapTest::CheckCapsule(hit,capsule,Move(capsule,{0.8f,0,0}));
    Require(hit.isCollision && Near(hit.mtv.x,-0.2f), "parallel capsules");
    OverlapTest::CheckCapsule(hit,capsule,{{-1,0,0},{1,0,0},0.5f});
    Require(hit.isCollision && Near(MiMath::Length(hit.mtv),1), "crossed axes");
    auto crossing = Move(capsule,MiMath::Multiply(hit.mtv,1.001f));
    OverlapTest::CheckCapsule(hit,crossing,{{-1,0,0},{1,0,0},0.5f});
    Require(!hit.isCollision, "crossed axis correction separates");
    OverlapTest::CheckCapsule(hit,{{0,0,0},{0,0,0},0.5f},{{0.8f,0,0},{0.8f,0,0},0.5f});
    Require(hit.isCollision && Near(hit.mtv.x,-0.2f), "both capsules degenerate to spheres");

    const CollisionBoxShape box{{0,0,0},{2,2,2},{0,0,0,1}};
    OverlapTest::CheckCapsuleOBB(hit,Move(capsule,{1.3f,0,0}),box);
    Require(hit.isCollision && Near(hit.mtv.x,0.2f), "box face");
    OverlapTest::CheckCapsuleOBB(hit,Move(capsule,{1.4f,0,1.4f}),box);
    Require(!hit.isCollision, "box corner does not use expanded AABB approximation");
    CheckSeparated(capsule,box);
    CheckSeparated({{-3,0,0},{3,0,0},0.5f},box);
    TransformComponent rotated;
    rotated.SetEulerAngle({0.3f,0.7f,0.2f});
    const CollisionBoxShape rotatedBox{{3,2,1},{2,2,2},rotated.GetRotation()};
    auto rotatedCapsule = Move(capsule,{1.3f,0,0});
    rotatedCapsule.pointA = MiMath::Add(rotatedBox.center,MiMath::RotateVector(rotatedBox.rotation,rotatedCapsule.pointA));
    rotatedCapsule.pointB = MiMath::Add(rotatedBox.center,MiMath::RotateVector(rotatedBox.rotation,rotatedCapsule.pointB));
    OverlapTest::CheckCapsuleOBB(hit,rotatedCapsule,rotatedBox);
    Require(hit.isCollision && Near(MiMath::Length(hit.mtv),0.2f), "rotated OBB contact");
    CheckSeparated(rotatedCapsule,rotatedBox);
    std::mt19937 random(42);
    std::uniform_real_distribution<float> value(-2.5f,2.5f);
    for (int i=0;i<2000;++i) {
        CollisionCapsuleShape sample{{value(random),value(random),value(random)},
            {value(random),value(random),value(random)},0.5f};
        CheckSeparated(sample,box);
    }

    TransformComponent rayTransform;
    CapsuleColliderComponent rayCollider;
    rayCollider.SetHeight(3);
    RaycastHit ray;
    RaycastTest::CheckRayCapsule(ray,{-2,0,0},{2,0,0},2,&rayTransform,&rayCollider);
    Require(ray.hit && Near(ray.hitDistance,1.5f) && Near(ray.hitNormal.x,-1), "ray side with non-unit direction");
    RaycastTest::CheckRayCapsule(ray,{0,3,0},{0,-1,0},1.5f,&rayTransform,&rayCollider);
    Require(ray.hit && Near(ray.hitDistance,1.5f) && Near(ray.hitNormal.y,1), "axial cap at max distance");
    RaycastTest::CheckRayCapsule(ray,{0,0,0},{0,1,0},2,&rayTransform,&rayCollider);
    Require(ray.hit && Near(ray.hitDistance,1.5f), "inside origin uses exterior exit");
    RaycastTest::CheckRayCapsule(ray,{0,0,0},{0,1,0},1,&rayTransform,&rayCollider);
    Require(!ray.hit, "inside exit beyond ray length");
    RaycastTest::CheckRayCapsule(ray,{-2,0,0},{0,0,0},10,&rayTransform,&rayCollider);
    Require(!ray.hit, "zero ray direction");
    RaycastTest::CheckRayCapsule(ray,{-2,0,0.5f},{1,0,0},3,&rayTransform,&rayCollider);
    Require(ray.hit && Near(ray.hitDistance,2), "tangent ray");
    rayCollider.SetHeight(1);
    RaycastTest::CheckRayCapsule(ray,{-2,0,0},{1,0,0},3,&rayTransform,&rayCollider);
    Require(ray.hit && Near(ray.hitDistance,1.5f), "ray degenerate sphere");

    TransformComponent moving, stationary;
    CapsuleColliderComponent movingCapsule;
    BoxColliderComponent obstacle;
    obstacle.SetScale({2,2,2});
    moving.SetPrevPosition({1.4f,0,0}); moving.SetPosition({1.3f,0,0});
    SweepTest::CheckCapsule(hit,&moving,&movingCapsule,&stationary,&obstacle);
    Require(hit.isCollision && Near(hit.mtv.x,0.2f), "sweep correction at current position");
    moving.SetPosition({2,0,0});
    SweepTest::CheckCapsule(hit,&moving,&movingCapsule,&stationary,&obstacle);
    Require(!hit.isCollision, "sweep allows moving away");
    // Current-position overlap must not report the previous position's overlap.
    OverlapTest::CheckCapsuleOBB(hit,
        CollisionShape::CreateCapsule(&moving,&movingCapsule,moving.GetPosition()),box);
    Require(!hit.isCollision, "current shape ignores previous overlap");
    std::cout << "PASS: " << checks << " capsule collision checks\n";
}
