#pragma once
#include <algorithm>
#include "Engine/Asset/Schema/field_master.h"
#include "Engine/Asset/DataAsset/data_asset.h"
#include "Engine/Asset/DataAsset/data_asset_type_id.h"
#include "Engine/Editor/Schema/field_editor.h"
#include "Engine/Editor/Schema/enum_field_editor.h"

namespace EnemyApproachSettings {
    struct Data {
        // === 距離設定 ===
        float stopDistance = 2.0f;
        float restartDistanceMargin = 0.3f;
        // === 移動速度設定 ===
        float moveSpeedMultiplier = 1.0f;
        // === 経路再探索設定 ===
        float repathInterval = 0.5f;
        float targetMoveThreshold = 0.5f;
        float minRepathInterval = 0.1f;
        float pathRetryInterval = 1.0f;
        int maxPathFailures = 3;
        // === 立ち往生検知設定 ===
        float stuckCheckInterval = 1.0f;
        float minProgressDistance = 0.1f;
    };

    inline void Sanitize(Data& data) {
        data.stopDistance = (std::max)(0.0f, data.stopDistance);
        data.restartDistanceMargin = (std::max)(0.0f, data.restartDistanceMargin);
        data.moveSpeedMultiplier = (std::max)(0.0f, data.moveSpeedMultiplier);
        data.minRepathInterval = (std::max)(0.01f, data.minRepathInterval);
        data.repathInterval = (std::max)(data.minRepathInterval, data.repathInterval);
        data.targetMoveThreshold = (std::max)(0.01f, data.targetMoveThreshold);
        data.pathRetryInterval = (std::max)(data.minRepathInterval, data.pathRetryInterval);
        data.maxPathFailures = (std::max)(1, data.maxPathFailures);
        data.stuckCheckInterval = (std::max)(0.01f, data.stuckCheckInterval);
        data.minProgressDistance = (std::max)(0.001f, data.minProgressDistance);
    }
    inline const auto& GetSchema() {
        static const auto schema = FieldSchema{
            // === Distance Settings ===
            MakeHeaderField("Distance Settings"),
            MakeField("stopDistance", "Stop Distance", &Data::stopDistance, DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 100.0f }),
            MakeField("restartDistanceMargin", "Restart Distance Margin", &Data::restartDistanceMargin, DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 100.0f }),
            // === Move Speed Settings ===
            MakeHeaderField("Move Speed Settings"),
            MakeField("moveSpeedMultiplier", "Move Speed Multiplier", &Data::moveSpeedMultiplier, DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 100.0f }),
            // === Repath Settings ===
            MakeHeaderField("Repath Settings"),
            MakeField("repathInterval", "Repath Interval (s)", &Data::repathInterval, DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 100.0f }),
            MakeField("targetMoveThreshold", "Target Move Threshold", &Data::targetMoveThreshold, DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 100.0f }),
            MakeField("minRepathInterval", "Min Repath Interval (s)", &Data::minRepathInterval, DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 100.0f }),
            MakeField("pathRetryInterval", "Path Retry Interval (s)", &Data::pathRetryInterval, DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 100.0f }),
            MakeField("maxPathFailures", "Max Path / Stuck Failures", &Data::maxPathFailures, DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 100.0f }),
            // === Stuck Check Settings ===
            MakeHeaderField("Stuck Check Settings"),
            MakeField("stuckCheckInterval", "Stuck Check Interval (s)", &Data::stuckCheckInterval, DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 100.0f }),
            MakeField("minProgressDistance", "Min Progress Distance", &Data::minProgressDistance, DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 100.0f }),
        };
        return schema;
    }
}

class EnemyApproachSettingsAsset : public DataAsset {
    EnemyApproachSettings::Data m_data;
public:
    EnemyApproachSettingsAsset() : DataAsset(DataAssetTypeID::getTypeID<EnemyApproachSettingsAsset>(), "EnemyApproachSettingsAsset", 0) {}
    const EnemyApproachSettings::Data& GetData() const { return m_data; }
    std::unique_ptr<DataAsset> CreateDefaultInstance() const override { return std::make_unique<EnemyApproachSettingsAsset>(); }
    nlohmann::json SerializeData() const override { return FieldSerialization::SerializeFields(m_data, EnemyApproachSettings::GetSchema()); }
    bool DeserializeDataToApply(const nlohmann::json& jsonData) override {
        auto loaded = m_data;
        if (!FieldSerialization::DeserializeFields(jsonData, loaded, EnemyApproachSettings::GetSchema())) return false;
        EnemyApproachSettings::Sanitize(loaded);
        m_data = loaded;
        return true;
    }
    bool DrawDataOnEditor() override {
        const bool changed = FieldEditor::DrawFields(m_data, EnemyApproachSettings::GetSchema());
        if (changed) EnemyApproachSettings::Sanitize(m_data);
        return changed;
    }
};
