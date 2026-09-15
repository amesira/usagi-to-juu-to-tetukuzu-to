#pragma once
#include "Engine/Component/behavior_component.h"
#include "result_ui_settings_asset.h"
#include "Game/ControllerBehavior/Result/game_result.h"
#include "Game/PresBehavior/UI/ui_handle.h"
#include "Game/PresBehavior/UI/Title/title_ui_presentation.h"
class ResultUiBehavior : public BehaviorComponent {
private:
     struct Row { 
         UiHandle label, gauge, status; 
         float fill = 0, velocity = 0; 
     };
     enum class Phase { Waves, Status, Total, CompleteBonus, TimeBonus, Rank, RankReward, Menu };
     const ResultUiSettingsAsset* m_asset = nullptr;
     ResultUiSettings::Data m_settings;

     GameResult m_result = {};
     ResultScoring::Score m_score;
     std::vector<Row> m_rows;
     UiHandle m_total, m_complete, m_time, m_rank, m_rankRewardMaterial, m_rankRewardSuffix;
     UiHandle m_title, m_retry, m_selectionBackground;
     TitleUiSelectionMotion m_selectionMotion;
     struct Shake {
         UiHandle handle;
         DirectX::XMFLOAT2 origin;
         float age = 0;
     };
     std::vector<Shake> m_shakes;
     Phase m_phase = Phase::Waves;
     size_t m_row = 0;
     float m_age = 0;
     int m_selection = 0;
     bool m_created = false, m_hasResult = false;
     std::uint64_t m_revision = 0;
     DirectX::XMFLOAT2 m_screen = {};

     void ApplyLayout();
     void ApplySelectionPosition();
     void ShowWithShake(UiHandle handle);
     void UpdateShakes(float dt);
     void Advance(Phase phase) { m_phase = phase; m_age = 0; }

public:
     void Setup(const ResultUiSettingsAsset* asset) { m_asset = asset; }
     void SetResult(const GameResult& result, const ResultScoring::Score& score);
     bool IsMenuReady() const { return m_created && m_phase == Phase::Menu; }
     int GetSelection() const { return m_selection; }
     void SetSelection(int selection);
     void DestroyWidgets();
     void Start() override;
     void Update() override;
     void DrawComponentInspector() override;
};
