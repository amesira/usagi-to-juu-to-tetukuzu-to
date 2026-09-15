//===================================================
// rect_transform_component.h
// 
// Author：Miu Kitamura
// Date  ：2025/11/18
//===================================================
#ifndef RECT_TRANSFORM_COMPONENT_H
#define RECT_TRANSFORM_COMPONENT_H
#include "Engine/Core/component.h"
#include "ui_chromatic_echo.h"

#include "Engine/Device/direct3d.h"
using namespace DirectX;

class RectTransformComponent : public Component {
private:
    DirectX::XMFLOAT3   m_position = { 0.0f,0.0f,0.0f };
    DirectX::XMFLOAT3   m_rotation = { 0.0f,0.0f,0.0f };
    DirectX::XMFLOAT3   m_scaling = { 1.0f,1.0f,1.0f };

    // UIの演出等に使う、描画時に適用する変換行列
    DirectX::XMFLOAT4X4 m_presentationTransform = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};

    UiChromaticEcho m_chromaticEcho;
public:
    void SetChromaticEcho(const UiChromaticEcho& echo) { m_chromaticEcho = echo; }
    const UiChromaticEcho& GetChromaticEcho() const { return m_chromaticEcho; }
    void SetPresentationTransform(const DirectX::XMFLOAT4X4& transform) { m_presentationTransform = transform; }
    const DirectX::XMFLOAT4X4& GetPresentationTransform() const { return m_presentationTransform; }
    void    SetPosition(DirectX::XMFLOAT3 position) { m_position = position; }
    void    SetRotation(DirectX::XMFLOAT3 rotation) { m_rotation = rotation; }
    void    SetScaling(DirectX::XMFLOAT3 scaling) { m_scaling = scaling; }

    DirectX::XMFLOAT3   GetPosition()const { return m_position; }
    DirectX::XMFLOAT3   GetRotation()const { return m_rotation; }
    DirectX::XMFLOAT3   GetScaling()const { return m_scaling; }

};


#endif