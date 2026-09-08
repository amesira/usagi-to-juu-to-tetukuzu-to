#pragma once
#include "player_ui_settings_asset.h"
#include <algorithm>
#include "Utility/mi_math.h"

namespace PlayerUiPerspective 
{
    using namespace DirectX;

    /// @brief UIの奥行的表示演出用の変換行列を作成する
    inline DirectX::XMFLOAT4X4 MakeTransform(
        const PlayerUiSettings::PerspectiveSettings& settings,
        DirectX::XMFLOAT2 anchor, DirectX::XMFLOAT2 currentAnchor, DirectX::XMFLOAT2 screenSize)
    {
        XMFLOAT4X4 result;
        XMStoreFloat4x4(&result, XMMatrixIdentity());
        if (!settings.enabled || screenSize.x <= 0 || screenSize.y <= 0) return result;

        const XMFLOAT2 center = {
            screenSize.x * settings.vanishingPoint.x,
            screenSize.y * settings.vanishingPoint.y
        };

        // 画面中心からの距離に応じて傾きを計算
        float pitch = -(anchor.y - center.y) * 2 / screenSize.y * settings.tiltDegrees;
        pitch = MiMath::Clamp(pitch, -settings.maxTiltDegrees, settings.maxTiltDegrees);
        pitch = XMConvertToRadians(pitch);

        float yaw = (anchor.x - center.x) * 2 / screenSize.x * settings.tiltDegrees;
        yaw = MiMath::Clamp(yaw, -settings.maxTiltDegrees, settings.maxTiltDegrees);
        yaw = XMConvertToRadians(yaw);

        if (pitch == 0 && yaw == 0) return result;

        // カメラ距離から遠近感を表す射影行列を作成
        const float distance = (std::max)(settings.cameraDistance, 100.0f);
        const XMMATRIX projection = XMMatrixSet(
            1,0,0,0, 0,1,0,0, 0,0,0,1 / distance, 0,0,0,1);

        // 傾きから作成した回転行列に射影行列を掛けることで、値だけのz座標を奥行き変換に反映させる
        // ・UIのProjection行列は正射影であるため、奥行き変換の反映には行列の掛け算が必要
        // ・UIシェイク時の位置補正を反映するため、アンカー位置の差分を色々加算している
        XMStoreFloat4x4(&result,
            XMMatrixTranslation(-currentAnchor.x, -currentAnchor.y, 0)  // 位置を原点へ
            * XMMatrixRotationRollPitchYaw(pitch, yaw, 0)               // 傾き回転
            * XMMatrixTranslation(anchor.x - center.x, anchor.y - center.y, 0) // 位置を消失点へ
            * projection                                                // 射影行列で奥行き変換
            * XMMatrixTranslation(center.x + currentAnchor.x - anchor.x,// 位置を元の位置へ
                center.y + currentAnchor.y - anchor.y, 0));
        return result;
    }
}
