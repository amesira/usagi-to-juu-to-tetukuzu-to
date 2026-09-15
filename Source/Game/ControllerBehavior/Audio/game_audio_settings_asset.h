#pragma once
#include "Engine/Asset/DataAsset/data_asset.h"
#include "Engine/Asset/DataAsset/data_asset_type_id.h"
#include "Engine/Asset/Schema/field_master.h"
#include "Engine/Editor/Schema/field_editor.h"
#include <algorithm>
#include <cmath>
namespace GameAudioSettings {
struct Data {
    std::string titleBgm, battleBgm, battleBgm2, battleBgm3, resultBgm;
    std::string shotgun, shotgunAim, dualPistols, slashBurst, slashBurst2;
    std::string charge, chargeComplete;
    std::string playerRun, playerJump, playerLand;
    std::string enemyShot, enemySlash;
    std::string playerHit, enemyHit, recovery, getItem;
    std::string menuMove, menuConfirm, menuCancel;
    float masterVolume = 1, bgmVolume = .5f, seVolume = 1, fadeDuration = .5f;
};
inline const auto& GetSchema() {
    static const auto schema = FieldSchema{
        // === BGM ===
        MakeHeaderField("BGM"),
        MakeField("titleBgm", "Title BGM (PCM WAV)", &Data::titleBgm),
        MakeField("battleBgm", "Battle BGM (PCM WAV)", &Data::battleBgm),
        MakeField("battleBgm2", "Battle BGM 2 (PCM WAV)", &Data::battleBgm2),
        MakeField("battleBgm3", "Battle BGM 3 (PCM WAV)", &Data::battleBgm3),
        MakeField("resultBgm", "Result BGM (PCM WAV)", &Data::resultBgm),

        // === SE ===
        MakeHeaderField("SE"),
        MakeField("shotgun", "Shotgun", &Data::shotgun),
        MakeField("shotgunAim", "Shotgun Aim", &Data::shotgunAim),
        MakeField("dualPistols", "Dual Pistols", &Data::dualPistols),
        MakeField("slashBurst", "Slash Burst", &Data::slashBurst),
        MakeField("slashBurst2", "Slash Burst 2", &Data::slashBurst2),
        MakeField("charge", "Charge", &Data::charge),
        MakeField("chargeComplete", "Charge Complete", &Data::chargeComplete),
        MakeField("playerRun", "Player Run", &Data::playerRun),
        MakeField("playerJump", "Player Jump", &Data::playerJump),
        MakeField("playerLand", "Player Land", &Data::playerLand),
        MakeField("enemyShot", "Enemy Shot", &Data::enemyShot),
        MakeField("enemySlash", "Enemy Slash", &Data::enemySlash),
        MakeField("playerHit", "Player Hit", &Data::playerHit),
        MakeField("enemyHit", "Enemy Hit", &Data::enemyHit),
        MakeField("recovery", "Recovery", &Data::recovery),
        MakeField("getItem", "Get Item", &Data::getItem),
        MakeField("menuMove", "Menu Move", &Data::menuMove),
        MakeField("menuConfirm", "Menu Confirm", &Data::menuConfirm),
        MakeField("menuCancel", "Menu Cancel", &Data::menuCancel),
        
        // === Volume ===
        MakeHeaderField("Volume"),
        MakeField("masterVolume", "Master Volume", &Data::masterVolume, DragFieldOptions{.dragSpeed=.01f, .minValue=0, .maxValue=1}),
        MakeField("bgmVolume", "BGM Volume", &Data::bgmVolume, DragFieldOptions{.dragSpeed=.01f, .minValue=0, .maxValue=1}),
        MakeField("seVolume", "SE Volume", &Data::seVolume, DragFieldOptions{.dragSpeed=.01f, .minValue=0, .maxValue=1}),
        MakeField("fadeDuration", "BGM Fade Duration (s)", &Data::fadeDuration, DragFieldOptions{.dragSpeed=.01f, .minValue=0, .maxValue=10})
    }; return schema;
}
inline void Sanitize(Data& data) {
    auto volume = [](float v) { return std::isfinite(v) ? std::clamp(v, 0.0f, 1.0f) : 0.0f; };
    data.masterVolume = volume(data.masterVolume); data.bgmVolume = volume(data.bgmVolume); data.seVolume = volume(data.seVolume);
    data.fadeDuration = std::isfinite(data.fadeDuration) ? std::clamp(data.fadeDuration, 0.0f, 10.0f) : .5f;
}
}
class GameAudioSettingsAsset : public DataAsset {
    GameAudioSettings::Data m_data;
public:
    GameAudioSettingsAsset() : DataAsset(DataAssetTypeID::getTypeID<GameAudioSettingsAsset>(), "GameAudioSettingsAsset", 0) {}
    const GameAudioSettings::Data& GetData() const { return m_data; }
    std::unique_ptr<DataAsset> CreateDefaultInstance() const override { return std::make_unique<GameAudioSettingsAsset>(); }
    nlohmann::json SerializeData() const override { return FieldSerialization::SerializeFields(m_data, GameAudioSettings::GetSchema()); }
    bool DeserializeDataToApply(const nlohmann::json& json) override {
        auto loaded = m_data;
        if (!FieldSerialization::DeserializeFields(json, loaded, GameAudioSettings::GetSchema())) return false;
        GameAudioSettings::Sanitize(loaded); m_data = loaded; return true;
    }
    bool DrawDataOnEditor() override {
        bool changed = FieldEditor::DrawFields(m_data, GameAudioSettings::GetSchema());
        if (changed) GameAudioSettings::Sanitize(m_data);
        return changed;
    }
};
