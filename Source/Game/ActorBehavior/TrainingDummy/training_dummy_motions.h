//---------------------------------------------------
// File  ：_/TrainingDummy/training_dummy_motions.h
// Date  ：2026/09/07
// Author：Miu Kitamura
// 
// ・TrainingDummyのアニメーション、モーションを管理するクラス
//---------------------------------------------------
#pragma once
#include <DirectXMath.h>
#include "Engine/Component/animation_component.h"

class GameObject;

class TrainingDummyMotions {
private:
    class TransformComponent* m_transform = nullptr;

    class ModelComponent* m_modelComponent = nullptr;
    class AnimationComponent* m_animationComponent = nullptr;
    
    std::vector<AnimationBlendTree2DNode> m_knockbackBlendTreeNodes;

    float m_motionSmoothTime = 0.05f;
    float m_endMotionSmoothTime = 0.2f;

    DirectX::XMFLOAT2 m_currentKnockbackParameter = {};
    DirectX::XMFLOAT2 m_targetKnockbackParameter = {};
    DirectX::XMFLOAT2 m_knockbackParameterVelocity = {};

    float m_knockbackMotionTimer = 0.0f;

public:
    void Initialize(GameObject* owner);
    void Update(float deltaTime);

    /// @brief ノックバックモーションを再生
    /// @param powerRate 0.0f ~ 1.0f
    void PlayKnockbackMotion(const DirectX::XMFLOAT3& direction, float powerRate, float duration);

};