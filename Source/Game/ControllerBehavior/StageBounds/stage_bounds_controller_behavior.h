#pragma once
#include "Engine/Component/behavior_component.h"
#include "stage_bounds_settings_asset.h"
class TransformComponent;
class RigidbodyComponent;
class StageBoundsControllerBehavior : public BehaviorComponent {
    const StageBoundsSettingsAsset* m_settingsAsset = nullptr;
    bool m_debugDraw = false;
public:
    ~StageBoundsControllerBehavior() override;
    void Setup(const StageBoundsSettingsAsset* asset);
    void Start() override;
    void Update() override;
    void DrawComponentInspector() override;
    StageBounds::Result Resolve(TransformComponent* transform, DirectX::XMFLOAT3 position) const;
    DirectX::XMFLOAT3 ConstrainVelocity(TransformComponent* transform, RigidbodyComponent* rigidbody, DirectX::XMFLOAT3 velocity, float dt) const;
    bool IsGrounded(TransformComponent* transform, float distance) const;
    static StageBoundsControllerBehavior* Find(TransformComponent* transform);
private:
    bool AppliesTo(TransformComponent* transform) const;
    static StageBounds::Shape GetShape(TransformComponent* transform);
};
