#pragma once

#include <DirectXMath.h>
#include <string>

#include "Engine/Asset/DataAsset/data_asset.h"
#include "Engine/Asset/DataAsset/data_asset_type_id.h"
#include "Engine/Asset/Schema/field_master.h"
#include "Engine/Editor/Schema/field_editor.h"

namespace TrainingDummyPrefabSettings
{
    struct Data {
        std::string modelAssetPath = "asset/Model/kakashi.fbx";

        DirectX::XMFLOAT3 scaling = { 1.0f, 1.0f, 1.0f };
        DirectX::XMFLOAT3 colliderCenter = { 0.0f, 1.0f, 0.0f };
        float colliderRadius = 0.5f;
        float colliderHeight = 2.0f;

        float maxHealth = 100.0f;
        float mass = 1.0f;
        DirectX::XMFLOAT3 friction = { 1.0f, 1.0f, 1.0f };
        float gravityScale = -9.8f;
    };

    inline static const auto& GetSchema()
    {
        static const auto& schema = FieldSchema{
            MakeHeaderField("Training Dummy Settings"),
            MakeField(
                "modelAssetPath",
                "Model Asset Path",
                &Data::modelAssetPath),
            MakeField(
                "scaling",
                "Scaling",
                &Data::scaling,
                DragFieldOptions{
                    .dragSpeed = 0.1f,
                    .minValue = 0.1f,
                    .maxValue = 10.0f }),

            // === Collider Settings ===
            MakeHeaderField("Collider Settings"),
            MakeField("colliderCenter", "Collider Center", &Data::colliderCenter,
                DragFieldOptions{
                    .dragSpeed = 0.1f,
                    .minValue = -10.0f,
                    .maxValue = 10.0f }),
            MakeField("colliderRadius", "Collider Radius", &Data::colliderRadius,
                DragFieldOptions{
                    .dragSpeed = 0.01f,
                    .minValue = 0.01f,
                    .maxValue = 10.0f }),
            MakeField("colliderHeight", "Collider Height", &Data::colliderHeight,
                DragFieldOptions{
                    .dragSpeed = 0.01f,
                    .minValue = 0.01f,
                    .maxValue = 10.0f }),

            // === Health Settings ===
            MakeHeaderField("Health Settings"),
            MakeField(
                "maxHealth",
                "Max Health",
                &Data::maxHealth,
                DragFieldOptions{
                    .dragSpeed = 1.0f,
                    .minValue = 1.0f,
                    .maxValue = 99999.0f }),

            MakeHeaderField("Physics Settings"),
            MakeField(
                "mass",
                "Mass",
                &Data::mass,
                DragFieldOptions{
                    .dragSpeed = 0.1f,
                    .minValue = 0.01f,
                    .maxValue = 1000.0f }),
            MakeField(
                "friction",
                "Friction",
                &Data::friction,
                DragFieldOptions{
                    .dragSpeed = 0.01f,
                    .minValue = 0.0f,
                    .maxValue = 1.0f }),
            MakeField(
                "gravityScale",
                "Gravity Scale",
                &Data::gravityScale,
                DragFieldOptions{
                    .dragSpeed = 0.1f,
                    .minValue = -100.0f,
                    .maxValue = 100.0f }),
        };
        return schema;
    }
}

class TrainingDummyPrefabSettingsAsset : public DataAsset {
private:
    TrainingDummyPrefabSettings::Data m_data;

    static constexpr std::string_view s_assetTypeName =
        "TrainingDummyPrefabSettingsAsset";
    static constexpr int s_supportedFormatVersion = 0;

public:
    TrainingDummyPrefabSettingsAsset()
        : DataAsset(
            DataAssetTypeID::getTypeID<TrainingDummyPrefabSettingsAsset>(),
            s_assetTypeName,
            s_supportedFormatVersion)
    {
    }

    ~TrainingDummyPrefabSettingsAsset() override = default;

    const TrainingDummyPrefabSettings::Data& GetData() const { return m_data; }

    std::unique_ptr<DataAsset> CreateDefaultInstance() const override
    {
        auto clone = std::make_unique<TrainingDummyPrefabSettingsAsset>();
        clone->SetHeader(AssetHeader{
            s_assetTypeName.data(),
            s_supportedFormatVersion,
            "" });
        clone->m_data = TrainingDummyPrefabSettings::Data{};
        return clone;
    }

    nlohmann::json SerializeData() const override
    {
        return FieldSerialization::SerializeFields(
            m_data,
            TrainingDummyPrefabSettings::GetSchema());
    }

    bool DeserializeDataToApply(const nlohmann::json& jsonData) override
    {
        TrainingDummyPrefabSettings::Data loaded = m_data;
        if (!FieldSerialization::DeserializeFields(
            jsonData,
            loaded,
            TrainingDummyPrefabSettings::GetSchema())) {
            return false;
        }

        m_data = loaded;
        return true;
    }

    bool DrawDataOnEditor() override
    {
        return FieldEditor::DrawFields(
            m_data,
            TrainingDummyPrefabSettings::GetSchema());
    }
};
