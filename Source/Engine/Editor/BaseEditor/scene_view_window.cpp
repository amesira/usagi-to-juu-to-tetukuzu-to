//===================================================
// scene_view_window.cpp
// 
// Author：Miu Kitamura
// Date  ：2026/03/12
//===================================================
#include "scene_view_window.h"
#include "Engine/Editor/editor_context.h"
#include "Engine/render_view.h"

void SceneViewWindow::Draw()
{
    const RenderView* renderView = m_editorContext->sceneRenderView;

    ID3D11ShaderResourceView* srv = renderView->colorBufferSRV.Get();
    float aspectRatio = renderView->aspectRatio;

    ImVec2 avail = ImGui::GetContentRegionAvail();
    if (avail.x / avail.y > aspectRatio) {
        avail.x = avail.y * aspectRatio;
    }
    else if (avail.x / avail.y < aspectRatio) {
        avail.y = avail.x / aspectRatio;
    }

    ImGui::Image((ImTextureID)(srv), avail);
}