//===================================================
// File  ：_/PresBehavior/UI/ui_handle.cpp
// Date  ：2026/09/07
// Author：Miu Kitamura
//===================================================
#include "ui_handle.h"

#include "Engine/Core/game_object.h"
#include "Engine/Core/scene_base.h"

#include "Engine/Component/transform_component.h"
#include "Engine/Component/rect_transform_component.h"
#include "Engine/Component/text_component.h"
#include "Engine/Component/image_component.h"
#include "Engine/Component/slider_component.h"

using namespace DirectX;

UiHandle::UiHandle(GameObject* gameObject)
{
    if (gameObject) {
        m_scene = gameObject->GetScene();
        m_gameObjectID = gameObject->GetID();
    }
}

bool UiHandle::IsValid() const
{
    if (!m_scene || m_gameObjectID == InvalidGameObjectID) return false;
    GameObject* gameObject = m_scene->GetGameObjectByID(m_gameObjectID);
    return gameObject != nullptr;
}

void UiHandle::SetActive(bool active)
{
    if (!IsValid()) return;
    GameObject* gameObject = m_scene->GetGameObjectByID(m_gameObjectID);
    if (gameObject) {
        gameObject->SetActive(active);
    }
}

void UiHandle::Destroy()
{
    if (!IsValid()) return;
    GameObject* gameObject = m_scene->GetGameObjectByID(m_gameObjectID);
    if (gameObject) {
        gameObject->Destroy();
    }
}

void UiHandle::SetColor(const DirectX::XMFLOAT3& color)
{
    if (!IsValid()) return;
    if (ImageComponent* image = GetImage()) {
        XMFLOAT4 newColor = {color.x, color.y, color.z, image->GetColor().w};
        image->SetColor(newColor);
    }
    if (TextComponent* text = GetText()) {
        XMFLOAT4 newColor = {color.x, color.y, color.z, text->GetColor().w};
        text->SetColor(newColor);
    }
    if (SliderComponent* slider = GetSlider()) { // 背景色もFill色も同じRGBにする
        XMFLOAT4 bgColor = { color.x, color.y, color.z, slider->GetBgColor().w };
        slider->SetBgColor(bgColor);
        XMFLOAT4 fillColor = {color.x, color.y, color.z, slider->GetFillColor().w};
        slider->SetFillColor(fillColor);
    }
}

/// @brief UIのアルファ値を設定する
void UiHandle::SetAlpha(float alpha)
{
    if (!IsValid()) return;
    if (ImageComponent* image = GetImage()) {
        XMFLOAT4 color = image->GetColor();
        color.w = alpha;
        image->SetColor(color);
    }
    if (TextComponent* text = GetText()) {
        XMFLOAT4 color = text->GetColor();
        color.w = alpha;
        text->SetColor(color);
    }
    if (SliderComponent* slider = GetSlider()) {
        XMFLOAT4 color = slider->GetBgColor();
        color.w = alpha;
        slider->SetBgColor(color);
        color = slider->GetFillColor();
        color.w = alpha;
        slider->SetFillColor(color);
    }
}

#pragma region RectTransform関連
void UiHandle::SetPosition(float x, float y)
{
    if (!IsValid()) return;
    if (RectTransformComponent* rectTransform = GetRectTransform()) {
        auto position = rectTransform->GetPosition();
        position.x = x;
        position.y = y;
        rectTransform->SetPosition(position); // 描画順のZは維持する。
    }
}

void UiHandle::SetSize(float width, float height)
{
    if (!IsValid()) return;
    if (RectTransformComponent* rectTransform = GetRectTransform()) {
        rectTransform->SetScaling({width, height, 1.0f});
    }
}
#pragma endregion

#pragma region Component取得関連
GameObject* UiHandle::GetGameObject() const
{
    if (!IsValid()) return nullptr;
    return m_scene->GetGameObjectByID(m_gameObjectID);
}

RectTransformComponent* UiHandle::GetRectTransform() const
{
    if (!IsValid()) return nullptr;
    GameObject* gameObject = m_scene->GetGameObjectByID(m_gameObjectID);
    if (gameObject) {
        return gameObject->GetComponent<RectTransformComponent>();
    }
    return nullptr;
}

TransformComponent* UiHandle::GetTransform() const
{
    if (!IsValid()) return nullptr;
    GameObject* gameObject = m_scene->GetGameObjectByID(m_gameObjectID);
    if (gameObject) {
        return gameObject->GetComponent<TransformComponent>();
    }
    return nullptr;
}

TextComponent* UiHandle::GetText() const
{
    if (!IsValid()) return nullptr;
    GameObject* gameObject = m_scene->GetGameObjectByID(m_gameObjectID);
    if (gameObject) {
        return gameObject->GetComponent<TextComponent>();
    }
    return nullptr;
}

ImageComponent* UiHandle::GetImage() const
{
    if (!IsValid()) return nullptr;
    GameObject* gameObject = m_scene->GetGameObjectByID(m_gameObjectID);
    if (gameObject) {
        return gameObject->GetComponent<ImageComponent>();
    }
    return nullptr;
}

SliderComponent* UiHandle::GetSlider() const
{
    if (!IsValid()) return nullptr;
    GameObject* gameObject = m_scene->GetGameObjectByID(m_gameObjectID);
    if (gameObject) {
        return gameObject->GetComponent<SliderComponent>();
    }
    return nullptr;
}
#pragma endregion
