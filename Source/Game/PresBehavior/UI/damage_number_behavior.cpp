//===================================================
// File  ：_/PresBehavior/UI/damage_number_behavior.cpp
// Date  ：2026/09/07
// Author：Miu Kitamura
//===================================================
#include "damage_number_behavior.h"

#include "Engine/Core/game_object.h"
#include "Engine/Component/transform_component.h"
#include "Engine/Component/text_component.h"

#include "Engine/Device/mi_fps.h"

void DamageNumberBehavior::Start()
{
    CacheComponents();
}

void DamageNumberBehavior::CacheComponents()
{
    GameObject* owner = GetOwner();
    if (!owner) return;

    m_transform = owner->GetComponent<TransformComponent>();
    m_text = owner->GetComponent<TextComponent>();
}

void DamageNumberBehavior::Show(float damage, const DirectX::XMFLOAT3& worldPosition, const DirectX::XMFLOAT4& color)
{
    // Startより先に呼ばれる場合も参照を取得
    CacheComponents();
    if (!m_transform || !m_text) return;

    m_damage = damage;
    m_startPosition = worldPosition;
    m_elapsedTime = 0.0f;
    m_isPlaying = true;
    m_transform->SetPosition(m_startPosition);

    m_text->SetText(std::to_string(static_cast<int>(damage)));
    m_text->SetColor(color);
}

void DamageNumberBehavior::Update()
{
    if (!m_isPlaying || !m_transform || !m_text) return;
    float deltaTime = FPS_GetDeltaTime();

    m_elapsedTime += deltaTime;

    UpdateMovement(deltaTime);
    UpdateFade(deltaTime);

    if (m_elapsedTime > 3.0f) {
        Hide();
    }
}

void DamageNumberBehavior::UpdateMovement(float deltaTime)
{
    if (m_elapsedTime < 0.3f) {
        // 上方向に移動する
        DirectX::XMFLOAT3 position = m_transform->GetPosition();
        position.y += deltaTime;
        m_transform->SetPosition(position);
    }
}

void DamageNumberBehavior::UpdateFade(float deltaTime)
{
    if (m_elapsedTime > 0.5f) {
        XMFLOAT4 color = m_text->GetColor();
        color.w -= deltaTime * 5.0f;
        if (color.w < 0.0f) color.w = 0.0f;
        m_text->SetColor(color);
    }
}

void DamageNumberBehavior::Hide()
{
    m_isPlaying = false;
    m_elapsedTime = 0.0f;

    GetOwner()->Destroy();
}

void DamageNumberBehavior::DrawComponentInspector()
{
    
}
