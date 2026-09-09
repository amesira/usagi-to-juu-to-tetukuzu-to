//===================================================
// debug_renderer.h
// 
// Author：Miu Kitamura
// Date  ：2025/12/04
//===================================================
#include "debug_renderer.h"

#include <vector>
#include "Engine/engine_service_locator.h"

static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;

static constexpr int NUM_VERTEX = 30000 * 2;
static ID3D11Buffer* g_pLineVertexBuffer = nullptr;
static std::vector<ShaderDefinitions::DebugLineVertex> g_LineVertices;

void DebugRenderer_Initialize()
{
    g_pDevice = Direct3D_GetDevice();
    g_pContext = Direct3D_GetDeviceContext();

    // ライン頂点バッファ生成
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(ShaderDefinitions::DebugLineVertex) * NUM_VERTEX; // 格納する最大頂点数
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    g_pDevice->CreateBuffer(&bd, NULL, &g_pLineVertexBuffer);

    // ライン頂点バッファの初期容量を設定
    g_LineVertices.reserve(NUM_VERTEX);
}

void DebugRenderer_Finalize()
{
    SAFE_RELEASE(g_pLineVertexBuffer);
}

void DebugRenderer_DrawFlush(const XMMATRIX& view, const XMMATRIX& projection)
{
    if (g_LineVertices.empty() || !g_pLineVertexBuffer) return;

    SetBlendState(BLENDSTATE_NONE);
    SetDepthState(DEPTHSTATE_DISABLE);

    // シェーダーをバインド
    EngineServiceLocator::BindShader(ShaderBase::DebugLine);

    // 定数バッファを更新
    EngineServiceLocator::UpdateCameraCB({ view, projection, {0.0f, 0.0f, 0.0f, 1.0f} });

    //----------------------------------------------------
    // 頂点バッファを更新
    //----------------------------------------------------
    {
        // 頂点バッファをロック
        D3D11_MAPPED_SUBRESOURCE msr;
        if (FAILED(g_pContext->Map(g_pLineVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr))) return;

        // 頂点バッファへの仮想ポインタを取得
        ShaderDefinitions::DebugLineVertex* v = (ShaderDefinitions::DebugLineVertex*)msr.pData;

        for (int i = 0; i < g_LineVertices.size(); i++) {
            v[i] = g_LineVertices[i];
        }

        // 頂点バッファのロックを解除
        g_pContext->Unmap(g_pLineVertexBuffer, 0);
    }

    // 頂点バッファを描画パイプラインに設定
    UINT stride = sizeof(ShaderDefinitions::DebugLineVertex);
    UINT offset = 0;
    g_pContext->IASetVertexBuffers(0, 1, &g_pLineVertexBuffer, &stride, &offset);

    // 頂点を2個ずつ組にして線分を描画
    g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    // ポリゴン描画命令発行
    g_pContext->Draw(static_cast<UINT>(g_LineVertices.size()), 0);
}

void DebugRenderer_ResetBuffer()
{
    g_LineVertices.clear();
}

void DebugRenderer_DrawLine(DirectX::XMFLOAT3 start, DirectX::XMFLOAT3 end, DirectX::XMFLOAT4 color)
{
    // 頂点バッファの容量オーバー防止
    if(g_LineVertices.size() + 2 > NUM_VERTEX) {
        return;
    }

    // ライン頂点をバッファに追加
    ShaderDefinitions::DebugLineVertex v = {};

    v.color = color;
    v.position = start;
    g_LineVertices.push_back(v);

    v.position = end;
    g_LineVertices.push_back(v);
}
