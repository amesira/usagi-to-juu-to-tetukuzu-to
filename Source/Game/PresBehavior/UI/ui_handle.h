//---------------------------------------------------
// File  ：_/PresBehavior/UI/ui_handle.h
// Date  ：2026/09/07
// Author：Miu Kitamura
// 
// ・UIのハンドルクラス。構造はEffectHandleとほぼ同じ
//---------------------------------------------------
#pragma once
#include <limits>

class UiHandle {
private:
    static constexpr unsigned int InvalidGameObjectID = (std::numeric_limits<unsigned int>::max)();

    class IScene* m_scene = nullptr;
    unsigned int m_gameObjectID = InvalidGameObjectID;

public:
    UiHandle() = default;
    UiHandle(class GameObject* gameObject);
    bool IsValid() const;

    void SetActive(bool active);
    void Destroy();

    void SetAlpha(float alpha);

    // RectTransformの位置とサイズを設定するための関数
    void SetPosition(float x, float y);
    void SetSize(float width, float height);

    class GameObject* GetGameObject() const;
    class RectTransformComponent* GetRectTransform() const;
    class TransformComponent* GetTransform() const;

    class TextComponent* GetText() const;
    class ImageComponent* GetImage() const;
    class SliderComponent* GetSlider() const;

};
