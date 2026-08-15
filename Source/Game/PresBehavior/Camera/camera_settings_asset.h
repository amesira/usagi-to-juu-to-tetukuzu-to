//---------------------------------------------------
// File  ：_/PresBehavior/Camera/camera_settings_asset.h
// Date  ：2026/08/15
// Author：Miu Kitamura
// 
// ・カメラの設定を保持するDataAssetのヘッダファイル
//---------------------------------------------------
#pragma once
#include "Engine/Asset/DataAsset/data_asset.h"
#include "Engine/Asset/DataAsset/data_asset_type_id.h"

#include "Engine/Asset/Schema/field_master.h"
#include "Engine/Editor/Schema/field_editor.h"

#include <DirectXMath.h>

namespace CameraSettings {
    using json = nlohmann::json;
    using namespace DirectX;

    /// @brief カメラの設定値を保持する構造体
    struct Data
    {
        // 基本構図
        XMFLOAT3 lookAtOffset = {};
        XMFLOAT3 lookAtLocalOffset = {};
        float lookAtHeight = 1.5f;
        float followDistance = 10.0f;
        float fov = 80.0f;

        // フォーカス
        float focusWeight = 0.5f;

        // 入力
        float mouseSensitivityX = 0.6f;
        float mouseSensitivityY = 0.6f;
        bool invertPitchInput = false;

        // 補間
        float rotationSmoothTime = 0.1f;
        float lookAtPositionSmoothTime = 0.1f;
        float cameraPositionSmoothTime = 0.1f;

        // 初期回転・制限
        float initialPitch = XMConvertToRadians(40.0f);
        float initialYaw = 0.0f;
        float maxPitch = XMConvertToRadians(50.0f);
        float minPitch = XMConvertToRadians(-5.0f);

        // シェイク
        float shakeFrequency = 35.0f;
    };

    /// @brief CameraSettingsのFieldSchemaを取得する
    inline static const auto& GetSchema()
    {
        using CameraSettingsData = Data;
        static const auto& schema = FieldSchema{
            // ここにFieldを追加していく
        };

        return schema;
    };
}

class CameraSettingsAsset : public DataAsset {
private:
    CameraSettings::Data m_data;

    static constexpr std::string_view s_assetTypeName = "CameraSettingsAsset";
    static constexpr int s_supportedFormatVersion = 0;

public:
    CameraSettingsAsset() : DataAsset(
        DataAssetTypeID::getTypeID<CameraSettingsAsset>(),
        s_assetTypeName, 
        s_supportedFormatVersion) {}
    ~CameraSettingsAsset() override = default;

    /// @brief カメラ設定のデータを取得する
    const CameraSettings::Data& GetData() const { return m_data; }

    // === DataAssetのオーバーライド関数 ===

    /// @brief CameraSettingsAssetのデフォルトインスタンスを作成する
    std::unique_ptr<DataAsset> CreateDefaultInstance() const override
    {
        auto clone = std::make_unique<CameraSettingsAsset>();
        clone->SetHeader(AssetHeader{ s_assetTypeName.data(), s_supportedFormatVersion, ""});
        clone->m_data = CameraSettings::Data {};
        return clone;
    }

    /// @brief CameraSettingsをJSON形式でシリアライズする
    nlohmann::json SerializeData() const override
    {
        nlohmann::json jsonData = FieldSerialization::SerializeFields(m_data, CameraSettings::GetSchema());
        return jsonData;
    }

    /// @brief CameraSettingsをJSON形式からデシリアライズする
    bool DeserializeDataToApply(const nlohmann::json& jsonData) override
    {
        CameraSettings::Data loaded = m_data;
        if (!FieldSerialization::DeserializeFields(jsonData, loaded, CameraSettings::GetSchema())) {
            return false;
        }
        // デシリアライズに成功した場合は、実データに反映する
        m_data = loaded;
        return true;
    }

    /// @brief CameraSettingsのデータをエディター上で描画する
    bool DrawDataOnEditor() override
    {
        bool changed = false;
        changed |= FieldEditor::DrawFields(m_data, CameraSettings::GetSchema());
        return changed;
    }
    
};