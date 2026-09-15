#include "Game/ControllerBehavior/Wave/wave_progress.h"
#include "Game/PresBehavior/UI/Result/result_ui_settings_asset.h"
#include "Game/PresBehavior/UI/Title/title_ui_presentation.h"
#include <cassert>
#include <fstream>

int main(int argc, char**) {
    GameResult result{{{1,55,50,true},{2,20,80,false}},40,false};
    auto score=ResultScoring::Calculate(result,{});
    assert(score.base==75 && score.total==75 && score.completeBonus==0 && score.timeBonus==0 && score.rank==1);
    result.completed=true;
    score=ResultScoring::Calculate(result,{});
    assert(score.completeBonus==500 && score.timeBonus==1300 && score.total==1875 && score.rank==5);
    result.elapsedTime=400;
    assert(ResultScoring::Calculate(result,{}).timeBonus==0);
    WaveSettings settings;settings.waveCount=2;settings.preparationDuration=0;settings.intermissionDuration=0;settings.cleanupDelay=0;
    WaveProgress progress;progress.Update(1,true,0,settings);
    progress.AddDefeatPoints(50,settings);progress.AddDefeatPoints(10,settings);
    assert(progress.state==WaveProgress::State::ClearImpact);
    progress.FinishClearImpact(settings);
    progress.MarkCleanupIssued();progress.Update(1,true,0,settings);
    assert(progress.results.size()==1 && progress.results[0].points==60 && progress.waveNumber==2);
    progress.RecordCurrentWave(false);progress.RecordCurrentWave(false);
    assert(progress.results.size()==2 && !progress.results.back().cleared);
    progress.AddDefeatPoints(80,settings);progress.FinishClearImpact(settings);progress.MarkCleanupIssued();progress.Update(1,true,0,settings);
    assert(progress.state==WaveProgress::State::Complete && progress.results.size()==2 && progress.results.back().cleared);
    ResultUiSettings::Data data;
    auto json=FieldSerialization::SerializeFields(data, ResultUiSettings::GetSchema());
    assert(json.contains("perspective") && json.contains("chromaticEcho"));
    assert(json.contains("rankSColor") && json.contains("rankDColor"));
    assert(json.contains("rankRewardMaterial") && json.contains("rankRewardSuffix"));
    assert(data.rankSRewardText == "ダイヤ" && data.rankCRewardText == "銅");
    assert(data.rankRewardSuffixText == "の鉄屑を贈呈！");
    assert(data.rankDRewardText == "ただ");
    assert(FieldSerialization::DeserializeFields(json, data, ResultUiSettings::GetSchema()));
    std::ifstream currentFile("asset/Data/result_ui_settings.data.json");
    auto currentJson = nlohmann::json::parse(currentFile);
    assert(FieldSerialization::DeserializeFields(currentJson.at("data"), data, ResultUiSettings::GetSchema()));
    const auto scoreCenter = UiLayoutSettings::ResolveGroupPosition(data.scorePlacement, {1920,1080});
    data.wavePlacement.position.x += 100;
    const auto sameScoreCenter = UiLayoutSettings::ResolveGroupPosition(data.scorePlacement, {1920,1080});
    assert(scoreCenter.x == sameScoreCenter.x && scoreCenter.y == sameScoreCenter.y);
    assert(data.title.position.x < data.retry.position.x);
    TitleUiSelectionMotion motion;
    motion.MoveTo(data.title.position, 0);
    motion.MoveTo(data.retry.position, 0.2f);
    motion.Update(0.1f);
    assert(motion.GetPosition().x > data.title.position.x && motion.GetPosition().x < data.retry.position.x);
    motion.MoveTo(data.title.position, 0.2f);
    motion.Update(0.2f);
    assert(motion.GetPosition().x == data.title.position.x);
    if (argc > 1) { std::ofstream file("asset/Data/result_ui_settings.data.json");
    file << nlohmann::json{{"data",json},{"formatVersion",0},{"name","result_ui_settings.data"},{"type","ResultUiSettingsAsset"}}.dump(4); }
}
