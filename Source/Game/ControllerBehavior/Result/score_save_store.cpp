#include "score_save_store.h"

#include "External/nlohmann/json.hpp"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>

namespace
{
    constexpr int SAVE_VERSION = 1;
    const std::filesystem::path SAVE_PATH = "save/score.json";

    ScoreSaveData g_saveData;
    bool g_loaded = false;

    ScoreRecord Sanitize(ScoreRecord record)
    {
        record.totalScore = (std::max)(record.totalScore, 0);
        record.rank = std::clamp(record.rank, 1, 5);
        if (!std::isfinite(record.clearTime) || record.clearTime < 0.0f) {
            record.clearTime = 0.0f;
        }
        return record;
    }

    nlohmann::json SerializeRecord(const ScoreRecord& record)
    {
        return {
            { "totalScore", record.totalScore },
            { "rank", record.rank },
            { "completed", record.completed },
            { "clearTime", record.clearTime },
        };
    }

    bool DeserializeRecord(const nlohmann::json& json, ScoreRecord& outRecord)
    {
        if (!json.is_object()) return false;

        ScoreRecord record;
        try {
            record.totalScore = json.at("totalScore").get<int>();
            record.rank = json.at("rank").get<int>();
            record.completed = json.at("completed").get<bool>();
            record.clearTime = json.at("clearTime").get<float>();
        }
        catch (const nlohmann::json::exception&) {
            return false;
        }

        outRecord = Sanitize(record);
        return true;
    }

    void EnsureLoaded()
    {
        if (!g_loaded) ScoreSaveStore::Load();
    }
}

bool ScoreSaveStore::Load()
{
    g_loaded = true;
    g_saveData = {};

    std::ifstream input(SAVE_PATH);
    if (!input) return false;

    try {
        nlohmann::json json;
        input >> json;
        if (!json.is_object() || json.value("version", 0) != SAVE_VERSION) return false;

        ScoreSaveData loaded;
        loaded.hasHighScore = json.value("hasHighScore", false);
        loaded.hasPreviousScore = json.value("hasPreviousScore", false);

        if (loaded.hasHighScore
            && (!json.contains("highScore") || !DeserializeRecord(json["highScore"], loaded.highScore))) {
            return false;
        }
        if (loaded.hasPreviousScore
            && (!json.contains("previousScore") || !DeserializeRecord(json["previousScore"], loaded.previousScore))) {
            return false;
        }

        g_saveData = loaded;
        return true;
    }
    catch (const nlohmann::json::exception&) {
        return false;
    }
}

bool ScoreSaveStore::Save()
{
    EnsureLoaded();

    std::error_code error;
    std::filesystem::create_directories(SAVE_PATH.parent_path(), error);
    if (error) return false;

    std::ofstream output(SAVE_PATH, std::ios::trunc);
    if (!output) return false;

    const nlohmann::json json = {
        { "version", SAVE_VERSION },
        { "hasHighScore", g_saveData.hasHighScore },
        { "hasPreviousScore", g_saveData.hasPreviousScore },
        { "highScore", SerializeRecord(g_saveData.highScore) },
        { "previousScore", SerializeRecord(g_saveData.previousScore) },
    };
    output << json.dump(4);
    return output.good();
}

void ScoreSaveStore::RegisterResult(const ScoreRecord& result)
{
    EnsureLoaded();

    const ScoreRecord sanitized = Sanitize(result);
    g_saveData.previousScore = sanitized;
    g_saveData.hasPreviousScore = true;

    if (!g_saveData.hasHighScore
        || sanitized.totalScore > g_saveData.highScore.totalScore) {
        g_saveData.highScore = sanitized;
        g_saveData.hasHighScore = true;
    }
}

const ScoreSaveData& ScoreSaveStore::Get()
{
    EnsureLoaded();
    return g_saveData;
}

