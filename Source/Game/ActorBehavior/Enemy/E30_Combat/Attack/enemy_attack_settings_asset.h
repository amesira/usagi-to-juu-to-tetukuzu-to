#pragma once
#include <algorithm>
#include <cmath>
#include "Engine/Asset/Schema/field_master.h"
#include "Engine/Asset/DataAsset/data_asset.h"
#include "Engine/Asset/DataAsset/data_asset_type_id.h"
#include "Engine/Editor/Schema/field_editor.h"

namespace EnemyAttackSettings {
    // 共通設定と各攻撃の仮の進行時間。実攻撃の接続後も調整に使用する。
    struct Data {
        float minDistance = 0.0f;
        float maxDistance = 2.0f;
        float windupDuration = 0.3f;
        float recoveryDuration = 0.5f;
        float restartCooldown = 1.0f;
        bool interruptibleWindup = true;
        bool interruptibleRecovery = true;
        float jumpDuration = 0.6f;
        float jumpHeight = 2.0f;
        float slashDuration = 0.3f;
        int shotCount = 3;
        float shotInterval = 0.3f;
    };

    inline void Sanitize(Data& data) {
        data.minDistance = std::isfinite(data.minDistance) ? (std::max)(0.0f, data.minDistance) : 0.0f;
        data.maxDistance = std::isfinite(data.maxDistance) ? (std::max)(0.0f, data.maxDistance) : 2.0f;
        data.windupDuration = std::isfinite(data.windupDuration) ? (std::max)(0.0f, data.windupDuration) : 0.3f;
        data.recoveryDuration = std::isfinite(data.recoveryDuration) ? (std::max)(0.0f, data.recoveryDuration) : 0.5f;
        data.restartCooldown = std::isfinite(data.restartCooldown) ? (std::max)(0.0f, data.restartCooldown) : 1.0f;
        data.jumpDuration = std::isfinite(data.jumpDuration) ? (std::max)(0.0f, data.jumpDuration) : 0.6f;
        data.jumpHeight = std::isfinite(data.jumpHeight) ? (std::max)(0.0f, data.jumpHeight) : 2.0f;
        data.slashDuration = std::isfinite(data.slashDuration) ? (std::max)(0.0f, data.slashDuration) : 0.3f;
        data.shotInterval = std::isfinite(data.shotInterval) ? (std::max)(0.0f, data.shotInterval) : 0.3f;
        data.maxDistance = (std::max)(data.minDistance, data.maxDistance);
        data.shotCount = (std::clamp)(data.shotCount, 1, 100);
        data.shotInterval = (std::max)(0.01f, data.shotInterval);
    }

    inline const auto& GetSchema() {
        static const auto schema = FieldSchema{
            MakeField("minDistance", "Min Distance", &Data::minDistance, DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 100.0f }),
            MakeField("maxDistance", "Max Distance", &Data::maxDistance, DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 100.0f }),
            MakeField("windupDuration", "Windup Duration", &Data::windupDuration, DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 100.0f }),
            MakeField("recoveryDuration", "Recovery Duration", &Data::recoveryDuration, DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 100.0f }),
            MakeField("restartCooldown", "Restart Cooldown", &Data::restartCooldown, DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 100.0f }),
            MakeField("interruptibleWindup", "Interruptible Windup", &Data::interruptibleWindup),
            MakeField("interruptibleRecovery", "Interruptible Recovery", &Data::interruptibleRecovery),
            MakeField("jumpDuration", "Jump Duration", &Data::jumpDuration, DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 100.0f }),
            MakeField("jumpHeight", "Jump Height", &Data::jumpHeight, DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 100.0f }),
            MakeField("slashDuration", "Slash Duration", &Data::slashDuration, DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 100.0f }),
            MakeField("shotCount", "Shot Count", &Data::shotCount, DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 100.0f }),
            MakeField("shotInterval", "Shot Interval", &Data::shotInterval, DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 100.0f }),
        };
        return schema;
    }
}

class EnemyAttackSettingsAsset : public DataAsset {
    EnemyAttackSettings::Data m_data;
public:
    EnemyAttackSettingsAsset() : DataAsset(DataAssetTypeID::getTypeID<EnemyAttackSettingsAsset>(), "EnemyAttackSettingsAsset", 0) {}
    const EnemyAttackSettings::Data& GetData() const { return m_data; }
    std::unique_ptr<DataAsset> CreateDefaultInstance() const override { return std::make_unique<EnemyAttackSettingsAsset>(); }
    nlohmann::json SerializeData() const override {
        return FieldSerialization::SerializeFields(m_data, EnemyAttackSettings::GetSchema());
    }
    bool DeserializeDataToApply(const nlohmann::json& jsonData) override {
        auto loaded = m_data;
        if (!FieldSerialization::DeserializeFields(jsonData, loaded, EnemyAttackSettings::GetSchema())) return false;
        EnemyAttackSettings::Sanitize(loaded);
        m_data = loaded;
        return true;
    }
    bool DrawDataOnEditor() override {
        const bool changed = FieldEditor::DrawFields(m_data, EnemyAttackSettings::GetSchema());
        if (changed) EnemyAttackSettings::Sanitize(m_data);
        return changed;
    }
};
