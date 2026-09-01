//---------------------------------------------------
// transform_constraint_behavior.h
// 
// Author：Miu Kitamura
// Date  ：2026/06/02
//---------------------------------------------------
#ifndef TRANSFORM_CONSTRAINT_BEHAVIOR_H
#define TRANSFORM_CONSTRAINT_BEHAVIOR_H
#include "Engine/Component/behavior_component.h"
#include "Engine/Device/direct3d.h"

using namespace DirectX;

class TransformComponent;

class TransformConstraintBehavior : public BehaviorComponent {
private:
    TransformComponent* m_transform = nullptr;

    // 制約の対象
    TransformComponent* m_target = nullptr;

    // 対象からのローカルTransform
    XMFLOAT3 m_localPosition = { 0.0f, 0.0f, 0.0f };
    XMFLOAT4 m_localRotation = { 0.0f, 0.0f, 0.0f, 1.0f };
    XMFLOAT3 m_localScaling = { 1.0f, 1.0f, 1.0f };

    bool m_considerRotation = true; // 回転を考慮するかどうか
    bool m_considerScaling = false; // スケーリングを考慮するかどうか

public:
    void Start() override;
    void Update() override;
    void DrawComponentInspector() override;

    /// @brief 現在のローカルTransformを追従先へ即時反映する
    void ApplyConstraint();

    // 制約の対象の設定・取得
    void SetTarget(TransformComponent* target) { m_target = target; }
    TransformComponent* GetTarget() const { return m_target; }

    // オフセットの設定・取得
    void SetOffset(const XMFLOAT3& offset) { SetLocalPosition(offset); }
    const XMFLOAT3& GetOffset() const { return GetLocalPosition(); }

    void SetLocalPosition(const XMFLOAT3& position) { m_localPosition = position; }
    const XMFLOAT3& GetLocalPosition() const { return m_localPosition; }
    void SetLocalRotation(const XMFLOAT4& rotation) { m_localRotation = rotation; }
    const XMFLOAT4& GetLocalRotation() const { return m_localRotation; }
    void SetLocalScaling(const XMFLOAT3& scaling) { m_localScaling = scaling; }
    const XMFLOAT3& GetLocalScaling() const { return m_localScaling; }

    // 回転を考慮するかどうかの設定・取得
    void SetConsiderRotation(bool consider) { m_considerRotation = consider; }
    bool GetConsiderRotation() const { return m_considerRotation; }
    // スケーリングを考慮するかどうかの設定・取得
    void SetConsiderScaling(bool consider) { m_considerScaling = consider; }
    bool GetConsiderScaling() const { return m_considerScaling; }

private:
    // 制約された位置の計算
    XMFLOAT3 CalculateConstrainedPosition() const;
    XMFLOAT4 CalculateConstrainedRotation() const;
    XMFLOAT3 CalculateConstrainedScaling() const;
};

#endif // TRANSFORM_CONSTRAINT_BEHAVIOR_H
