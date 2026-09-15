//---------------------------------------------------
// transparent_render_pass.h
// 
// Author：Miu Kitamura
// Date  ：2026/06/02
//---------------------------------------------------
#ifndef TRANSPARENT_RENDER_PASS_H
#define TRANSPARENT_RENDER_PASS_H
#include "Engine/Core/pass.h"

#include "Engine/Device/direct3d.h"

#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

class TextureResource;
class ModelResource;

class ParticleSystemComponent;
class LineRendererComponent;
class MeshEffectComponent;
class TransformComponent;

class TransparentRenderPass : public Pass {
private:
    ID3D11Device* m_pDevice = nullptr;
    ID3D11DeviceContext* m_pContext = nullptr;

    // デフォルトテクスチャ
    TextureResource* m_defaultTexture = nullptr;
    ModelResource* m_defaultModel = nullptr;

    ComPtr<ID3D11Buffer> m_pParticleVertexBuffer;
    ComPtr<ID3D11Buffer> m_pParticleInstanceBuffer;
    ComPtr<ID3D11Buffer> m_pLineVertexBuffer;
    ComPtr<ID3D11Buffer> m_pLineInstanceBuffer;

    class ShaderProgramResource* m_pMeshEffectShaderProgram = nullptr;
    class ConstantBufferResource* m_pMeshEffectConstantBuffer = nullptr;

public:
    void Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    void Finalize() override;
    void Process(IScene* pScene, const RenderView& view) override;

private:
    void DrawParticleSystem(ParticleSystemComponent& particleSystem, const RenderView& view);
    void DrawLineRenderer(LineRendererComponent& lineRenderer, const RenderView& view);
    void DrawMeshEffect(MeshEffectComponent& meshEffect, const RenderView& view, const TransformComponent& transform);

};

#endif // TRANSPARENT_RENDER_PASS_H
