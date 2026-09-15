//===================================================
// image_component.h
// 
// Author：Miu Kitamura
// Date  ：2025/11/18
//===================================================
#ifndef IMAGE_COMPONENT_H
#define IMAGE_COMPONENT_H
#include "Engine/Core/component.h"

#include "Engine/Device/direct3d.h"
using namespace DirectX;

#include "Engine/Graphics/texture_resource.h"
#include <algorithm>
#include <cmath>

class ImageComponent : public Component {
public:
    enum class FillMethod {
        None,
        Horizontal, // 左から右（Reverseで右から左）
        Vertical,   // 下から上（Reverseで上から下）
        RoundFill,  // 中心から扇形。開始角度0度は上、通常は時計回り
    };
    enum class WorldSpaceType {
        None,       // 通常表示
        Billboard,  // 常にカメラ方向を向く
        HD2D,       // Y軸回りのみカメラ方向を向く
    };

private:
    TextureResource*   m_pTextureResource = nullptr;
    XMFLOAT4           m_uvRect = { 0.0f,0.0f,1.0f,1.0f };
    XMFLOAT4           m_color = { 1.0f,1.0f,1.0f,1.0f };

    WorldSpaceType  m_worldSpaceType = WorldSpaceType::None;
    FillMethod m_fillMethod = FillMethod::None;
    float m_fillAmount = 1.0f;
    bool m_fillReverse = false;
    float m_fillStartAngleDegrees = 0.0f;

public:
    void    SetTextureResource(TextureResource* resource) { m_pTextureResource = resource; }
    void    SetUvRect(XMFLOAT4 uvRect) { m_uvRect = uvRect; }
    void    SetColor(XMFLOAT4 color) { m_color = color; }
    void SetFillMethod(FillMethod method) { m_fillMethod = method; }
    void SetFillAmount(float amount) {
        m_fillAmount = std::isfinite(amount) ? std::clamp(amount, 0.0f, 1.0f) : 0.0f;
    }
    void SetFillReverse(bool reverse) { m_fillReverse = reverse; }
    void SetFillStartAngleDegrees(float degrees) {
        m_fillStartAngleDegrees = std::isfinite(degrees) ? std::fmod(degrees, 360.0f) : 0.0f;
        if (m_fillStartAngleDegrees < 0) m_fillStartAngleDegrees += 360.0f;
    }
    float GetFillStartAngleDegrees() const { return m_fillStartAngleDegrees; }
    FillMethod GetFillMethod() const { return m_fillMethod; }
    float GetFillAmount() const { return m_fillAmount; }
    bool GetFillReverse() const { return m_fillReverse; }

    TextureResource* GetTextureResource()const { return m_pTextureResource; }
    XMFLOAT4   GetUvRect()const { return m_uvRect; }
    XMFLOAT4   GetColor()const { return m_color; }

   void    SetWorldSpaceType(WorldSpaceType type) { m_worldSpaceType = type; }
   WorldSpaceType  GetWorldSpaceType()const { return m_worldSpaceType; }
};

#endif
