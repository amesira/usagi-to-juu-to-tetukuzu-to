// level_asset.h
#pragma once
#include "Engine/Asset/i_asset.h"
#include "level_data.h"

class LevelAsset : public IAsset
{
private:
    LevelData m_data;

public:
    const LevelData& GetData() const { return m_data; }
    LevelData& GetData() { return m_data; }
};
