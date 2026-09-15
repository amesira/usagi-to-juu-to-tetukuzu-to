//===================================================
// health_behavior.cpp
// 
// Author：Miu Kitamura
// Date  ：2026/03/28
//===================================================
#include "health_behavior.h"

#include <algorithm>

#include "Engine/Core/game_object.h"
#include "Engine/Core/scene_interface.h"
#include "Engine/Component/slider_component.h"
#include "Engine/Component/transform_component.h"
#include "Game/Factory/ui_factory.h"
#include "External/ImGui/imgui.h"

#include "Game/ActorBehavior/transform_constraint_behavior.h"

HealthBehavior::~HealthBehavior()
{
    m_healthBar.Destroy();
}

void HealthBehavior::Start()
{
    CreateWorldHealthUi();
    UpdateWorldHealthUi();
}

void HealthBehavior::Update()
{
    UpdateWorldHealthUi();
}

void HealthBehavior::DrawComponentInspector()
{
    ImGui::DragFloat("Health", &m_health, 1.0f, 0.0f, m_maxHealth);
    ImGui::DragFloat("Max Health", &m_maxHealth, 1.0f, 1.0f, 9999.0f);
    if (m_health > m_maxHealth) {
        m_health = m_maxHealth;
    }
}

void HealthBehavior::TakeDamage(float damage)
{
    m_health = std::clamp(m_health - damage, 0.0f, m_maxHealth);
    if (m_onTakeDamage) {
        m_onTakeDamage(m_health, damage);
    }
}

/// @brief UIのオフセットを設定する
void HealthBehavior::SetUiOffset(const DirectX::XMFLOAT3& offset)
{
    m_uiOffset = offset;
    TransformConstraintBehavior* constraint = m_healthBar.GetGameObject()
        ? m_healthBar.GetGameObject()->GetComponent<TransformConstraintBehavior>()
        : nullptr;
    if (constraint) {
        constraint->SetOffset(offset);
    }
}

void HealthBehavior::CreateWorldHealthUi()
{
    GameObject* owner = GetOwner();
    if (!owner || m_healthBar.IsValid()) return;

    m_healthBar = UiFactory::CreateWorldUiSliderHandle(
        owner->GetScene(),
        { 0.2f, 0.2f, 0.2f, 1.0f },
        { 1.0f, 0.2f, 0.1f, 1.0f },
        GetHealthRate());

    if (TransformComponent* uiTransform = m_healthBar.GetTransform()) {
        uiTransform->SetScaling(m_uiScale);
    }
}

void HealthBehavior::UpdateWorldHealthUi()
{
    if (!m_healthBar.IsValid()) return;

    GameObject* owner = GetOwner();
    TransformComponent* ownerTransform = owner
        ? owner->GetComponent<TransformComponent>()
        : nullptr;
    TransformComponent* uiTransform = m_healthBar.GetTransform();
    if (!ownerTransform || !uiTransform) return;

    DirectX::XMFLOAT3 position = ownerTransform->GetPosition();
    position.x += m_uiOffset.x;
    position.y += m_uiOffset.y;
    position.z += m_uiOffset.z;
    uiTransform->SetPosition(position);

    if (SliderComponent* slider = m_healthBar.GetSlider()) {
        slider->SetValue(GetHealthRate());
    }
}
