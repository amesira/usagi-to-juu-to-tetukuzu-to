//---------------------------------------------------
// File  ：_/PresBehavior/UI/damage_number_behavior.h
// Date  ：2026/09/07
// Author：Miu Kitamura
// 
// ・ダメージ数値の表示・演出を管理するBehaviorComponent
//---------------------------------------------------
#pragma once
#include "Engine/Component/behavior_component.h"
#include <DirectXMath.h>

class TransformComponent;
class TextComponent;

// ワールド配置したダメージ数値の表示・演出を管理する
class DamageNumberBehavior : public BehaviorComponent {
private:
    TransformComponent* m_transform = nullptr;
    TextComponent* m_text = nullptr;

    float m_damage = 0.0f;
    DirectX::XMFLOAT3 m_startPosition = { 0.0f, 0.0f, 0.0f };

    float m_elapsedTime = 0.0f;
    bool m_isPlaying = false;

public:
    void Start() override;
    void Update() override;
    void DrawComponentInspector() override;

    void Show(float damage, const DirectX::XMFLOAT3& worldPosition, const DirectX::XMFLOAT4& color = { 1.0f, 1.0f, 1.0f, 1.0f});
    void Hide();

    bool IsPlaying() const { return m_isPlaying; }

private:
    void CacheComponents();
    void UpdateMovement(float deltaTime);
    void UpdateFade(float deltaTime);

};
