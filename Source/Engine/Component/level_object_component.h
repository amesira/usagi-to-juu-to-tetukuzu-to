#pragma once

#include "Engine/Core/component.h"
#include <string>

// LevelAssetから生成されたGameObjectであることを示すマーカー。
class LevelObjectComponent : public Component {
private:
    std::string m_levelObjectId;

public:
    void SetLevelObjectId(const std::string& id) { m_levelObjectId = id; }
    const std::string& GetLevelObjectId() const { return m_levelObjectId; }
};
