//===================================================
// ui_factory.cpp
// 
// Author：Miu Kitamura
// Date  ：2026/04/14
//===================================================
#include "ui_factory.h"
#include "Engine/Core/game_object.h"
#include "Engine/Core/scene_interface.h"

// component
#include "Engine/Component/rect_transform_component.h"
#include "Engine/Component/transform_component.h"
#include "Engine/Component/image_component.h"
#include "Engine/Component/text_component.h"
#include "Engine/Component/slider_component.h"

#include "Engine/engine_service_locator.h"

// UIイメージ生成
GameObject* UiFactory::CreateUiImage(IScene* scene, const std::wstring& texturePath)
{
    if (!scene || !TEXTURE_REPOSITORY) return nullptr;
    TextureResource* textureResource = TEXTURE_REPOSITORY->GetTextureResource(texturePath);
    if (!textureResource) return nullptr;
    GameObject* uiImage = scene->CreateGameObject();
    if (!uiImage) return nullptr;
    uiImage->SetName("UiImage");

    // component生成・登録
    RectTransformComponent* rectTransform = uiImage->AddComponent<RectTransformComponent>();
    ImageComponent* imageComp = uiImage->AddComponent<ImageComponent>();

    imageComp->SetTextureResource(textureResource);

    return uiImage;
}

// UIテキスト生成
GameObject* UiFactory::CreateUiText(IScene* scene, const std::u8string& text)
{
    if (!scene) return nullptr;
    GameObject* uiText = scene->CreateGameObject();
    if (!uiText) return nullptr;
    uiText->SetName("UiText");

    // component生成・登録
    RectTransformComponent* rectTransform = uiText->AddComponent<RectTransformComponent>();
    TextComponent* textComp = uiText->AddComponent<TextComponent>();
    textComp->SetText(text);
    textComp->SetFontSize(32);
    textComp->SetColor({ 1.0f, 0.5f, 0.5f, 1.0f });
    textComp->SetCenter(true);
    textComp->SetFontPath("asset/Font/Makinas-4-Square.otf");

    return uiText;
}

// UIスライダー生成
GameObject* UiFactory::CreateUiSlider(IScene* scene, const XMFLOAT4& bgColor, const XMFLOAT4& fillColor, float value)
{
    if (!scene) return nullptr;
    GameObject* uiSlider = scene->CreateGameObject();
    if (!uiSlider) return nullptr;
    uiSlider->SetName("UiSlider");

    // component生成・登録
    RectTransformComponent* rectTransform = uiSlider->AddComponent<RectTransformComponent>();
    SliderComponent* sliderComp = uiSlider->AddComponent<SliderComponent>();

    sliderComp->SetBgColor(bgColor);
    sliderComp->SetFillColor(fillColor);
    sliderComp->SetValue(value);

    return uiSlider;
}

UiHandle UiFactory::CreateUiImageHandle(IScene* scene, const std::wstring& texturePath)
{
    return UiHandle(CreateUiImage(scene, texturePath));
}

UiHandle UiFactory::CreateUiTextHandle(IScene* scene, const std::u8string& text)
{
    return UiHandle(CreateUiText(scene, text));
}

UiHandle UiFactory::CreateUiSliderHandle(IScene* scene, const XMFLOAT4& bgColor,
    const XMFLOAT4& fillColor, float value)
{
    return UiHandle(CreateUiSlider(scene, bgColor, fillColor, value));
}

UiHandle UiFactory::CreateWorldUiSliderHandle(IScene* scene, const XMFLOAT4& bgColor,
    const XMFLOAT4& fillColor, float value)
{
    if (!scene) return {};

    GameObject* uiSlider = scene->CreateGameObject();
    if (!uiSlider) return {};
    uiSlider->SetName("WorldUiSlider");

    uiSlider->AddComponent<TransformComponent>();
    SliderComponent* sliderComp = uiSlider->AddComponent<SliderComponent>();
    if (!sliderComp) {
        uiSlider->Destroy();
        return {};
    }

    sliderComp->SetBgColor(bgColor);
    sliderComp->SetFillColor(fillColor);
    sliderComp->SetValue(value);

    return UiHandle(uiSlider);
}

// UITransformセットアップ
bool UiFactory::SetupUiTransform(GameObject* uiElement, const XMFLOAT2& position, const XMFLOAT2& size)
{
    if (!uiElement) return false;
    RectTransformComponent* rectTransform = uiElement->GetComponent<RectTransformComponent>();
    if (!rectTransform) return false;

    rectTransform->SetPosition({ position.x, position.y, 0.0f });
    rectTransform->SetScaling({ size.x, size.y, 1.0f });

    return true;
}
