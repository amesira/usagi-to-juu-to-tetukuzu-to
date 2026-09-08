// UI rendering for screen-space and billboard world-space components.
#include "ui_render_pass.h"
#include "Engine/engine_service_locator.h"
#include "Engine/render_view.h"
#include <algorithm>

namespace {
    constexpr UINT MAX_UI_INSTANCES = 2048;

    template<class Batch, class MakeWorld>
    void DrawBatches(ID3D11DeviceContext* context, ID3D11Buffer* vertices,
        ID3D11Buffer* instances, const std::vector<Batch>& batches, MakeWorld makeWorld)
    {
        if (!vertices || !instances) return;
        ID3D11Buffer* buffers[] = { vertices, instances };
        UINT strides[] = { sizeof(UiVertex), sizeof(UiInstanceData) };
        UINT offsets[] = { 0, 0 };
        context->IASetVertexBuffers(0, 2, buffers, strides, offsets);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

        for (const auto& batch : batches) {
            if (!batch.shaderProgram || !batch.texture || batch.instances.empty()) continue;
            EngineServiceLocator::BindShader(batch.shaderProgram);
            context->PSSetShaderResources(0, 1, &batch.texture);

            // Split large batches instead of discarding instances beyond buffer capacity.
            for (size_t start = 0; start < batch.instances.size(); start += MAX_UI_INSTANCES) {
                const UINT count = static_cast<UINT>((std::min)(
                    batch.instances.size() - start, static_cast<size_t>(MAX_UI_INSTANCES)));

                D3D11_MAPPED_SUBRESOURCE mapped = {};
                if (FAILED(context->Map(instances, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) break;

                auto* data = static_cast<UiInstanceData*>(mapped.pData);

                for (UINT i = 0; i < count; ++i) {
                    const auto& command = batch.instances[start + i];
                    data[i].world = makeWorld(command);
                    data[i].color = command.color;
                    data[i].uvRect = command.uvRect;
                    data[i].roundFill = command.roundFill;
                }
                context->Unmap(instances, 0);
                context->DrawInstanced(4, count, 0, 0);
            }
        }
        ID3D11Buffer* emptyBuffer = nullptr;
        UINT zero = 0;
        context->IASetVertexBuffers(1, 1, &emptyBuffer, &zero, &zero);
    }
}

void UIRenderPass::Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    m_pDevice = pDevice;
    m_pContext = pContext;
    m_batches.reserve(2048);
    m_batches3D.reserve(32);
    m_collectorImage.Initialize();
    m_collectorSlider.Initialize();
    m_collectorFont.Initialize();

    const UiVertex vertices[] = {
        { { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f } },
        { {  0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f } },
        { { -0.5f,  0.5f, 0.0f }, { 0.0f, 1.0f } },
        { {  0.5f,  0.5f, 0.0f }, { 1.0f, 1.0f } },
    };
    D3D11_BUFFER_DESC desc = {};
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.ByteWidth = sizeof(vertices);
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA initialData = {};
    initialData.pSysMem = vertices;
    m_pDevice->CreateBuffer(&desc, &initialData, &m_pVertexBuffer);

    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.ByteWidth = sizeof(UiInstanceData) * MAX_UI_INSTANCES;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    m_pDevice->CreateBuffer(&desc, nullptr, &m_pInstanceBuffer);
}

void UIRenderPass::Finalize()
{
    m_collectorImage.Finalize();
    m_collectorSlider.Finalize();
    m_collectorFont.Finalize();
    SAFE_RELEASE(m_pVertexBuffer);
    SAFE_RELEASE(m_pInstanceBuffer);
}

void UIRenderPass::Process(IScene* pScene, const RenderView& view)
{
    if (!pScene) return;
    SetBlendState(BLENDSTATE_ALFA);
    SetDepthState(DEPTHSTATE_DISABLE);
    m_batches.clear();
    m_collectorImage.CollectDrawBatches2D(pScene, m_batches);
    m_collectorSlider.CollectDrawBatches2D(pScene, m_batches);
    m_collectorFont.CollectDrawBatches2D(pScene, m_batches);
    std::stable_sort(m_batches.begin(), m_batches.end(), [](const auto& a, const auto& b) {
        return a.orderInLayer < b.orderInLayer;
    });
    DrawBatches(m_pContext, m_pVertexBuffer, m_pInstanceBuffer, m_batches,
        [](const UiDrawCommand::DrawCommand2DInstance& instance) {
            return XMMatrixScaling(instance.size.x, instance.size.y, 1.0f)
                * XMMatrixRotationZ(instance.angleZ)
                * XMMatrixTranslation(instance.position.x, instance.position.y, 0.0f);
        });
}

void UIRenderPass::Process3D(IScene* pScene, const RenderView& view)
{
    if (!pScene) return;
    SetBlendState(BLENDSTATE_ALFA);
    SetDepthState(DEPTHSTATE_NOWRITE);
    SetRasterizerState(RASTERIZERSTATE_CULL_NONE);
    EngineServiceLocator::UpdateCameraCB({ view.viewMatrix, view.projectionMatrix,
        { view.eyePosition.x, view.eyePosition.y, view.eyePosition.z, 1.0f } });

    m_batches3D.clear();
    m_collectorImage.CollectDrawBatches3D(pScene, m_batches3D);
    m_collectorSlider.CollectDrawBatches3D(pScene, m_batches3D);
    m_collectorFont.CollectDrawBatches3D(pScene, m_batches3D);

    XMMATRIX billboard = XMMatrixInverse(nullptr, view.viewMatrix);
    billboard.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
    DrawBatches(m_pContext, m_pVertexBuffer, m_pInstanceBuffer, m_batches3D,
        [&billboard](const UiDrawCommand::DrawCommand3DInstance& instance) {
            return UiDrawCommand::MakeBillboardWorld(instance, billboard);
        });
    SetRasterizerState(RASTERIZERSTATE_CULL_BACK);
}
