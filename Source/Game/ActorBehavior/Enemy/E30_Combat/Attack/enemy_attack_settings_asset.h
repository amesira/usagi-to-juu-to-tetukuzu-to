// enemy_attack_settings_asset.h
// 2026/09/11
#pragma once
#include <algorithm>
#include <cmath>
#include <string>
#include <DirectXMath.h>
#include "Engine/Asset/Schema/field_master.h"
#include "Engine/Asset/DataAsset/data_asset.h"
#include "Engine/Asset/DataAsset/data_asset_type_id.h"
#include "Engine/Editor/Schema/field_editor.h"

namespace EnemyAttackSettings {
    // 共通設定と各攻撃の仮の進行時間。実攻撃の接続後も調整に使用する。
    struct Data {
        // === 攻撃の射程 ===
        float minDistance = 0.0f;
        float maxDistance = 2.0f;
        // === 攻撃の進行時間 ===
        float windupDuration = 0.3f;
        float recoveryDuration = 0.5f;
        float restartCooldown = 1.0f;
        // === 進行中の割り込み可否 ===
        bool interruptibleWindup = true;
        bool interruptibleRecovery = true;

        // === Melee用の追加設定 ===
        float jumpDuration = 0.6f;
        float jumpGravity = 9.8f;
        float slashDuration = 0.3f;
        float slashBurstTime = 0.15f;
        float slashDamage = 10.0f;
        DirectX::XMFLOAT3 slashBoxSize = { 3.0f, 2.0f, 2.0f };
        DirectX::XMFLOAT3 slashBoxOffset = { 0.0f, 1.0f, 1.0f };
        std::string slashEffectAssetPath = "asset/MeshEffect/player_slash_burst_1_effect.mesh_effect.json";
        DirectX::XMFLOAT3 slashEffectPosition = {};
        DirectX::XMFLOAT3 slashEffectRotation = {}; // ローカルEuler角（度）
        float slashEffectScale = 1.0f;

        // === Ranged用の追加設定 ===
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
        data.jumpGravity = std::isfinite(data.jumpGravity) ? (std::max)(0.0f, data.jumpGravity) : 9.8f;
        data.slashDuration = std::isfinite(data.slashDuration) ? (std::max)(0.0f, data.slashDuration) : 0.3f;
        data.slashBurstTime = std::isfinite(data.slashBurstTime)
            ? (std::clamp)(data.slashBurstTime, 0.0f, data.slashDuration) : data.slashDuration * 0.5f;
        const auto nonNegative = [](float value, float fallback) {
            return std::isfinite(value) ? (std::max)(0.0f, value) : fallback;
        };
        data.slashDamage = nonNegative(data.slashDamage, 10.0f);
        data.slashEffectScale = nonNegative(data.slashEffectScale, 1.0f);
        data.slashBoxSize.x = (std::max)(0.01f, nonNegative(data.slashBoxSize.x, 3.0f));
        data.slashBoxSize.y = (std::max)(0.01f, nonNegative(data.slashBoxSize.y, 2.0f));
        data.slashBoxSize.z = (std::max)(0.01f, nonNegative(data.slashBoxSize.z, 2.0f));
        const auto finiteVector = [](DirectX::XMFLOAT3& value) {
            if (!std::isfinite(value.x)) value.x = 0.0f;
            if (!std::isfinite(value.y)) value.y = 0.0f;
            if (!std::isfinite(value.z)) value.z = 0.0f;
        };
        finiteVector(data.slashBoxOffset);
        finiteVector(data.slashEffectPosition);
        finiteVector(data.slashEffectRotation);
        data.shotInterval = std::isfinite(data.shotInterval) ? (std::max)(0.0f, data.shotInterval) : 0.3f;
        data.maxDistance = (std::max)(data.minDistance, data.maxDistance);
        data.shotCount = (std::clamp)(data.shotCount, 1, 100);
        data.shotInterval = (std::max)(0.01f, data.shotInterval);
    }

    inline const auto& GetSchema() {
        static const auto schema = FieldSchema{
            // === 攻撃の射程 ===
            MakeHeaderField("Attack Range"),
            MakeField("minDistance", "Min Distance", &Data::minDistance, DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 100.0f }),
            MakeField("maxDistance", "Max Distance", &Data::maxDistance, DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 100.0f }),
            // === 攻撃の進行時間 ===
            MakeHeaderField("Attack Timing"),
            MakeField("windupDuration", "Windup Duration", &Data::windupDuration, DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 100.0f }),
            MakeField("recoveryDuration", "Recovery Duration", &Data::recoveryDuration, DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 100.0f }),
            MakeField("restartCooldown", "Restart Cooldown", &Data::restartCooldown, DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 100.0f }),
            // === 進行中の割り込み可否 ===
            MakeHeaderField("Interruptibility"),
            MakeField("interruptibleWindup", "Interruptible Windup", &Data::interruptibleWindup),
            MakeField("interruptibleRecovery", "Interruptible Recovery", &Data::interruptibleRecovery),
            // === Melee用の追加設定 ===
            MakeHeaderField("Melee Attack Settings"),
            MakeField("jumpDuration", "Jump Duration", &Data::jumpDuration, DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 100.0f }),
            MakeField("jumpGravity", "Jump Gravity", &Data::jumpGravity, DragFieldOptions{.dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 100.0f }),
            MakeField("slashDuration", "Slash Duration", &Data::slashDuration, DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 100.0f }),
            // === Slashの判定と演出 ===
            MakeHeaderField("Slash Burst"),
            MakeField("slashBurstTime", "Burst Time (s)", &Data::slashBurstTime, DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 1000.0f }),
            MakeField("slashDamage", "Damage", &Data::slashDamage, DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 1000.0f }),
            MakeField("slashBoxSize", "Hit Box Size", &Data::slashBoxSize, DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 1000.0f }),
            MakeField("slashBoxOffset", "Hit Box Local Offset", &Data::slashBoxOffset, DragFieldOptions{ .dragSpeed = 0.01f, .minValue = -360.0f, .maxValue = 1000.0f }),
            MakeHeaderField("Slash Effect"),
            MakeField("slashEffectAssetPath", "Effect Asset Path", &Data::slashEffectAssetPath),
            MakeField("slashEffectPosition", "Local Position", &Data::slashEffectPosition, DragFieldOptions{ .dragSpeed = 0.01f, .minValue = -360.0f, .maxValue = 1000.0f }),
            MakeField("slashEffectRotation", "Local Rotation (degrees)", &Data::slashEffectRotation, DragFieldOptions{ .dragSpeed = 0.01f, .minValue = -360.0f, .maxValue = 1000.0f }),
            MakeField("slashEffectScale", "Scale", &Data::slashEffectScale, DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 1000.0f }),
            // === Ranged用の追加設定 ===
            MakeHeaderField("Ranged Attack Settings"),
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
