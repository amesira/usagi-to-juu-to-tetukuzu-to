//---------------------------------------------------
// File  ：_/TrainingDummy/training_dummy_motions.h
// Date  ：2026/09/07
// Author：Miu Kitamura
// 
// ・TrainingDummyのアニメーション、モーションを管理するクラス
//---------------------------------------------------
#pragma once
#include <DirectXMath.h>
#include "Game/ActorBehavior/Base/ReactionEffects/hit_stop_sequence_task.h"
#include "Engine/Component/animation_component.h"

class GameObject;

class TrainingDummyMotions {
private:
    class TransformComponent* m_transform = nullptr;

    class ModelComponent* m_modelComponent = nullptr;
    class AnimationComponent* m_animationComponent = nullptr;

    std::vector<AnimationBlendTree2DNode> m_knockbackBlendTreeNodes;

    float m_springAngularFrequency = 24.0f; // 揺れの角周波数（rad/s）
    float m_springDampingRatio = 0.25f;     // 減衰比。0より大きく1未満で振動しながら収束

    DirectX::XMFLOAT2 m_currentKnockbackParameter = {};
    DirectX::XMFLOAT2 m_targetKnockbackParameter = {};
    DirectX::XMFLOAT2 m_knockbackParameterVelocity = {};

    HitStopTask m_hitStopTask;
    bool m_holdingHitPose = false;

    float m_knockbackMotionTimer = 0.0f;

public:
    void Initialize(GameObject* owner);
    void Update(float deltaTime);
    void PlaySlashHitMotion(const DirectX::XMFLOAT3& direction, float powerRate, float duration);
    void CancelHitStop();

    /// @brief ノックバックモーションを再生
    /// @param powerRate 0.0f ~ 1.0f
    void PlayKnockbackMotion(const DirectX::XMFLOAT3& direction, float powerRate, float duration);

};