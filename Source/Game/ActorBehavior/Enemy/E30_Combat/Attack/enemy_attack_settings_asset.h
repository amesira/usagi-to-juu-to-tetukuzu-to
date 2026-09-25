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
        float windupShakeMagnitude = 0.08f;
        float windupShakeFrequency = 25.0f;
        DirectX::XMFLOAT3 windupShakeAxis = { 1.0f, 0.35f, 1.0f };
        float slashDuration = 0.3f;
        float slashBurstTime = 0.15f;
        float slashDamage = 10.0f;
        DirectX::XMFLOAT3 slashBoxSize = { 3.0f, 2.0f, 2.0f };
        DirectX::XMFLOAT3 slashBoxOffset = { 0.0f, 1.0f, 1.0f };
        std::string slashEffectAssetPath = "asset/MeshEffect/player_slash_burst_1_effect.mesh_effect.json";
        DirectX::XMFLOAT3 slashEffectPosition = {};
        DirectX::XMFLOAT3 slashEffectRotation = {}; // ローカルEuler角（度）
        float slashEffectScale = 1.0f;
        DirectX::XMFLOAT3 windupEffectPosition = {};
        DirectX::XMFLOAT3 windupEffectRotation = {}; // ローカルEuler角（度）

        // === Ranged用の追加設定 ===
        int shotCount = 3;
        float shotInterval = 0.3f;
        float firstShotDelay = 0.1f; // 各単発クリップ開始から発射までの秒数
        float shotPlaybackSpeed = 1.0f;
        std::string leftMuzzleBoneName = "Gun.L";
        std::string rightMuzzleBoneName = "Gun.R";
        float projectileSpeed = 30.0f;
        float projectileRadius = 0.2f;
        float projectileLifeTime = 5.0f;
        float projectileDamage = 10.0f;
        float projectileSpawnForwardOffset = 0.2f;
        float targetHeightOffset = 1.0f;
    };

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
            MakeField("windupShakeMagnitude", "Windup Shake Magnitude", &Data::windupShakeMagnitude, DragFieldOptions{ .dragSpeed = 0.001f, .minValue = 0.0f, .maxValue = 10.0f }),
            MakeField("windupShakeFrequency", "Windup Shake Frequency", &Data::windupShakeFrequency, DragFieldOptions{ .dragSpeed = 0.1f, .minValue = 0.01f, .maxValue = 100.0f }),
            MakeField("windupShakeAxis", "Windup Shake Axis", &Data::windupShakeAxis, DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 1.0f }),
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
            MakeField("windupEffectPosition", "Windup Effect Local Position", 
                &Data::windupEffectPosition, DragFieldOptions{.dragSpeed = 0.01f, .minValue = -360.0f, .maxValue = 1000.0f }),
            MakeField("windupEffectRotation", "Windup Effect Local Rotation (degrees)",
                &Data::windupEffectRotation, DragFieldOptions{.dragSpeed = 0.01f, .minValue = -360.0f, .maxValue = 1000.0f }),

            // === Ranged用の追加設定 ===
            MakeHeaderField("Ranged Attack Settings"),
            MakeField("firstShotDelay", "Shot Clip Fire Delay (s)", &Data::firstShotDelay, DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 100.0f }),
            MakeField("shotPlaybackSpeed", "Shot Playback Speed", &Data::shotPlaybackSpeed, DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.01f, .maxValue = 100.0f }),
            MakeField("shotCount", "Shot Count", &Data::shotCount, DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 100.0f }),
            MakeField("shotInterval", "Shot Interval", &Data::shotInterval, DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 100.0f }),
            MakeField("leftMuzzleBoneName", "Left Muzzle Bone", &Data::leftMuzzleBoneName),
            MakeField("rightMuzzleBoneName", "Right Muzzle Bone", &Data::rightMuzzleBoneName),
            MakeField("projectileSpeed", "Projectile Speed", &Data::projectileSpeed, DragFieldOptions{ .dragSpeed = 0.1f, .minValue = 0.0f, .maxValue = 1000.0f }),
            MakeField("projectileRadius", "Projectile Radius", &Data::projectileRadius, DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 100.0f }),
            MakeField("projectileLifeTime", "Projectile Life Time", &Data::projectileLifeTime, DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 100.0f }),
            MakeField("projectileDamage", "Projectile Damage", &Data::projectileDamage, DragFieldOptions{ .dragSpeed = 0.1f, .minValue = 0.0f, .maxValue = 10000.0f }),
            MakeField("projectileSpawnForwardOffset", "Spawn Forward Offset", &Data::projectileSpawnForwardOffset, DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 100.0f }),
            MakeField("targetHeightOffset", "Target Height Offset", &Data::targetHeightOffset, DragFieldOptions{ .dragSpeed = 0.01f, .minValue = -100.0f, .maxValue = 100.0f }),
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
        m_data = loaded;
        return true;
    }
    bool DrawDataOnEditor() override {
        const bool changed = FieldEditor::DrawFields(m_data, EnemyAttackSettings::GetSchema());
        return changed;
    }
};
