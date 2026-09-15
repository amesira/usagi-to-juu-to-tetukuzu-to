#pragma once

struct ScoreRecord
{
    int totalScore = 0;
    int rank = 1;
    bool completed = false;
    float clearTime = 0.0f;
};

struct ScoreSaveData
{
    ScoreRecord highScore;
    ScoreRecord previousScore;
    bool hasHighScore = false;
    bool hasPreviousScore = false;
};

namespace ScoreSaveStore
{
    // 保存ファイルを読み込む。存在しない、または不正な場合は未記録として扱う。
    bool Load();
    // 現在の保存データをsave/score.jsonへ書き込む。
    bool Save();
    // 前回スコアを更新し、必要ならハイスコアも更新する。
    void RegisterResult(const ScoreRecord& result);
    // 初回アクセス時は自動的にLoadする。
    const ScoreSaveData& Get();
}

