//----------------------------------------------------
// collector_image.h
// 
// Author：Miu Kitamura
// Date  ：2025/11/18
//----------------------------------------------------
#ifndef COLLECTOR_IMAGE_H
#define COLLECTOR_IMAGE_H
#include <vector>
#include "ui_draw_command.h"
class IScene;
class ShaderProgramResource;

class CollectorImage {
private:
    ShaderProgramResource* m_pDefaultUiShader = nullptr;
    
public:
    void Initialize();
    void Finalize();

    void CollectDrawBatches2D(IScene* pScene, std::vector<UiDrawCommand::DrawBatch2D>& outBatches);
    void CollectDrawBatches3D(IScene* pScene, std::vector<UiDrawCommand::DrawBatch3D>& outBatches);

};

#endif
