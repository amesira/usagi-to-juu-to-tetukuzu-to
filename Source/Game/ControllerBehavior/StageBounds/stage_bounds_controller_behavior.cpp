#include "stage_bounds_controller_behavior.h"
#include "Game/ControllerBehavior/game_controller_locator.h"
#include "Engine/Core/game_object.h"
#include "Engine/Component/transform_component.h"
#include "Engine/Component/collider_component.h"
#include "Engine/Component/rigidbody_component.h"
#include "Engine/Editor/LevelEditor/behavior_detail_view.h"
#include "Utility/debug_renderer.h"
StageBoundsControllerBehavior::~StageBoundsControllerBehavior() {
    if (Game::StageBounds() == this) GameControllerLocator::s_stageBoundsController = nullptr;
}
void StageBoundsControllerBehavior::Setup(const StageBoundsSettingsAsset* asset) {
    m_settingsAsset = asset;
    GameControllerLocator::s_stageBoundsController = this;
}
void StageBoundsControllerBehavior::Start() { GameControllerLocator::s_stageBoundsController = this; }
bool StageBoundsControllerBehavior::AppliesTo(TransformComponent* t) const {
    return m_settingsAsset && GetEnable() && GetOwner() && GetOwner()->GetActive()
        && t && t->GetOwner() && t->GetOwner()->GetScene() == GetOwner()->GetScene();
}
StageBoundsControllerBehavior* StageBoundsControllerBehavior::Find(TransformComponent* t) {
    auto* bounds = Game::StageBounds();
    return bounds && bounds->AppliesTo(t) ? bounds : nullptr;
}
StageBounds::Shape StageBoundsControllerBehavior::GetShape(TransformComponent* t) {
    StageBounds::Shape shape;
    if (!t || !t->GetOwner()) return shape;
    auto* capsule = t->GetOwner()->GetComponent<CapsuleColliderComponent>();
    if (!capsule) return shape;
    using namespace DirectX;
    const auto rotation = t->GetRotation();
    auto q = XMLoadFloat4(&rotation);
    auto center = capsule->GetCenter();
    XMStoreFloat3(&shape.offset, XMVector3Rotate(XMLoadFloat3(&center), q));
    XMFLOAT3 axis;
    XMStoreFloat3(&axis, XMVector3Rotate(XMVectorSet(0, 1, 0, 0), q));
    const float radius = capsule->GetRadius();
    const float segment = (std::max)(0.0f, capsule->GetHeight() * .5f - radius);
    shape.extent = {radius + std::abs(axis.x) * segment, radius + std::abs(axis.y) * segment, radius + std::abs(axis.z) * segment};
    return shape;
}
StageBounds::Result StageBoundsControllerBehavior::Resolve(TransformComponent* t, DirectX::XMFLOAT3 p) const {
    return AppliesTo(t) ? StageBounds::Resolve(m_settingsAsset->GetData(), GetShape(t), p) : StageBounds::Result{p};
}
DirectX::XMFLOAT3 StageBoundsControllerBehavior::ConstrainVelocity(TransformComponent* t, RigidbodyComponent* rb, DirectX::XMFLOAT3 velocity, float dt) const {
    if (!AppliesTo(t) || !rb || !rb->GetEnable() || rb->GetIsKinematic()) return velocity;
    const auto friction = rb->GetIsGrounded() ? rb->GetFriction() : rb->GetAirFriction();
    return StageBounds::ConstrainVelocity(m_settingsAsset->GetData(), GetShape(t), t->GetPosition(), velocity,
        friction, rb->GetGravityScale() * rb->GetMass(), dt);
}
bool StageBoundsControllerBehavior::IsGrounded(TransformComponent* t, float distance) const {
    return AppliesTo(t) && StageBounds::IsGrounded(m_settingsAsset->GetData(), GetShape(t), t->GetPosition(), distance);
}
void StageBoundsControllerBehavior::Update() {
    if (!m_debugDraw || !m_settingsAsset || Game::StageBounds() != this) return;
    const auto& s = m_settingsAsset->GetData();
    DirectX::XMFLOAT3 p[8];
    for (int i = 0; i < 8; ++i) p[i] = {(i & 1) ? s.max.x : s.min.x, (i & 2) ? s.max.y : s.min.y, (i & 4) ? s.max.z : s.min.z};
    for (int i = 0; i < 8; ++i) for (int bit : {1, 2, 4}) if (!(i & bit)) DebugRenderer_DrawLine(p[i], p[i | bit], {0, 1, 1, 1});
}
void StageBoundsControllerBehavior::DrawComponentInspector() {
    if (BehaviorDetailView::BeginSection(this, "Stage Bounds")) {
        ImGui::Text("Settings: %s", m_settingsAsset ? "asset/Data/stage_bounds_settings.data.json" : "None");
        ImGui::Checkbox("Draw Bounds", &m_debugDraw);
        ImGui::TextUnformatted("Edit limits in the StageBoundsSettingsAsset.");
    }
    BehaviorDetailView::EndSection();
}
