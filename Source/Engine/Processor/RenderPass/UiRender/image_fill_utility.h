#pragma once
#include "Engine/Component/image_component.h"
#include "ui_draw_command.h"
#include <cmath>

namespace ImageFillUtility {
    inline DirectX::XMFLOAT4 GetRoundFill(const ImageComponent& image) {
        if (image.GetFillMethod() != ImageComponent::FillMethod::RoundFill) return {};
        return {1.0f, image.GetFillAmount(),
            DirectX::XMConvertToRadians(image.GetFillStartAngleDegrees()),
            image.GetFillReverse() ? -1.0f : 1.0f};
    }

    // 元のImage/Transformを変更せず、切り抜き後のサイズ・UV・中心差分を計算する。
    // UIローカル座標はX右向き、Y下向き。
    inline bool Calculate(const ImageComponent& image, DirectX::XMFLOAT2& size,
        DirectX::XMFLOAT4& uvRect, DirectX::XMFLOAT2& offset)
    {
        offset = {};
        const auto method = image.GetFillMethod();
        if (method == ImageComponent::FillMethod::None) return true;
        const float amount = image.GetFillAmount();
        if (amount <= 0.0f) return false;
        const float removed = 1.0f - amount;
        if (method == ImageComponent::FillMethod::Horizontal) {
            offset.x = size.x * removed * (image.GetFillReverse() ? 0.5f : -0.5f);
            size.x *= amount;
            if (image.GetFillReverse()) uvRect.x += uvRect.z * removed;
            uvRect.z *= amount;
        }
        else if (method == ImageComponent::FillMethod::Vertical) {
            offset.y = size.y * removed * (image.GetFillReverse() ? -0.5f : 0.5f);
            size.y *= amount;
            if (!image.GetFillReverse()) uvRect.y += uvRect.w * removed;
            uvRect.w *= amount;
        }
        return true;
    }

    inline bool Apply(const ImageComponent& image, UiDrawCommand::DrawCommand2DInstance& instance)
    {
        instance.roundFill = GetRoundFill(image);
        DirectX::XMFLOAT2 offset;
        if (!Calculate(image, instance.size, instance.uvRect, offset)) return false;
        const float cosine = std::cos(instance.angleZ);
        const float sine = std::sin(instance.angleZ);
        instance.position.x += offset.x * cosine - offset.y * sine;
        instance.position.y += offset.x * sine + offset.y * cosine;
        return true;
    }

    inline bool Apply(const ImageComponent& image, UiDrawCommand::DrawCommand3DInstance& instance)
    {
        DirectX::XMFLOAT2 size = { instance.scale.x, instance.scale.y };
        instance.roundFill = GetRoundFill(image);
        DirectX::XMFLOAT2 offset;
        if (!Calculate(image, size, instance.uvRect, offset)) return false;
        instance.scale.x = size.x;
        instance.scale.y = size.y;
        instance.offset.x += offset.x;
        instance.offset.y += offset.y;
        return true;
    }
}
