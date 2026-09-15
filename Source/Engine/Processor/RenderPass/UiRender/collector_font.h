#pragma once
#include "ui_draw_command.h"

class IScene;
class ShaderProgramResource;
class FontRepository;

class CollectorFont {
private:
    ShaderProgramResource* m_pFontShader = nullptr;
    FontRepository* m_fontRepository = nullptr;

public:
    void Initialize();
    void Finalize();
    void CollectDrawBatches2D(IScene* pScene, std::vector<UiDrawCommand::DrawBatch2D>& outBatches);
    void CollectDrawBatches3D(IScene* pScene, std::vector<UiDrawCommand::DrawBatch3D>& outBatches);

private:
    int DecodeUtf8(const char8_t** text_ptr);
};