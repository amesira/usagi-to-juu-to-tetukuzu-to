//===================================================
// shadow_map_pass.cpp
//
// Author: Miu Kitamura
// Date  : 2026/04/03
//===================================================
#include "shadow_map_pass.h"
#include "Engine/Core/scene_interface.h"

#include "Engine/Component/model_component.h"
#include "Engine/Component/sprite_renderer_component.h"
#include "Engine/Component/transform_component.h"

#include "Engine/Processor/RenderPass/RenderUtility/model_render_utility.h"
#include "Engine/Processor/RenderPass/RenderUtility/sprite_render_utility.h"
#include "Engine/render_view.h"

#include "Engine/Graphics/shader_definitions.h"

#include "Utility/mi_math.h"
#include "Engine/engine_service_locator.h"

#define TEXTURE_REPOSITORY EngineServiceLocator::GetTextureRepository()
#define MATERIAL_REPOSITORY EngineServiceLocator::GetMaterialRepository()
#define SHADER_MANAGER EngineServiceLocator::Shader()
#define SHADER_REPOSITORY EngineServiceLocator::GetShaderRepository()

void ShadowMapPass::Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    m_pDevice = pDevice;
    m_pContext = pContext;

    m_defaultTexture = TEXTURE_REPOSITORY->GetTextureResource(L"asset\\Texture\\white.bmp");

    // シャドウマップ用のライト定数バッファを生成し、シェーダープログラムにバインド
    m_shadowLightCB = SHADER_REPOSITORY->GenerateConstantBufferResource(
        "ShadowLightBuffer",
        11,
        sizeof(XMMATRIX),
        true,
        true,
        ConstantBufferUsage::Default);
    SHADER_REPOSITORY->AddConstantBufferToShaderProgram(SHADER_BASE_NAMES[static_cast<size_t>(ShaderBase::Lit)], m_shadowLightCB);
    SHADER_REPOSITORY->AddConstantBufferToShaderProgram(SHADER_BASE_NAMES[static_cast<size_t>(ShaderBase::SkinnedLit)], m_shadowLightCB);
    SHADER_REPOSITORY->AddConstantBufferToShaderProgram(SHADER_BASE_NAMES[static_cast<size_t>(ShaderBase::SpriteLit)], m_shadowLightCB);

    // シャドウマップ用の深度バッファと対応するビューを生成
    constexpr UINT SHADOW_MAP_SIZE = 2048;
    Direct3D_CreateDepthBuffer(
        depthBufferTexture.GetAddressOf(), 
        depthBufferDSV.GetAddressOf(), 
        depthBufferSRV.GetAddressOf(), 
        SHADOW_MAP_SIZE,
        SHADOW_MAP_SIZE
    );

    // シャドウマップ参照用の比較サンプラーを生成（ピクセルシェーダーのs1へバインド）
    // COMPARISON_LESS_EQUALを使用することで、深度値が比較対象の値以下であれば1.0、それ以外は0.0を返すようになる。
    // BORDERなので、シャドウマップの外側は1.0（影が落ちない）となる。
    {
        D3D11_SAMPLER_DESC shadowSamplerDesc = {};
        shadowSamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
        shadowSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
        shadowSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
        shadowSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
        shadowSamplerDesc.BorderColor[0] = 1.0f;
        shadowSamplerDesc.BorderColor[1] = 1.0f;
        shadowSamplerDesc.BorderColor[2] = 1.0f;
        shadowSamplerDesc.BorderColor[3] = 1.0f;
        shadowSamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
        shadowSamplerDesc.MinLOD = 0.0f;
        shadowSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
        m_pDevice->CreateSamplerState(&shadowSamplerDesc, m_shadowComparisonSampler.GetAddressOf());
    }

    // スプライトをシャドウマップへ書き込むための頂点バッファを生成
    {
        D3D11_BUFFER_DESC bd = {};
        bd.Usage = D3D11_USAGE_DYNAMIC;
        bd.ByteWidth = sizeof(ShaderDefinitions::SpriteVertex) * 4;
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        m_pDevice->CreateBuffer(&bd, NULL, m_spriteVertexBuffer.GetAddressOf());
    }
}

void ShadowMapPass::Finalize()
{

}

void ShadowMapPass::Process(IScene* pScene, const RenderView& view)
{
    // シャドウマップのレンダリング設定
    SetBlendState(BLENDSTATE_NONE);
    SetDepthState(DEPTHSTATE_ENABLE);
    SetRasterizerState(RASTERIZERSTATE_CULL_BACK);
    SetSamplerState(SAMPLERSTATE_POINT_WRAP);

    {
        constexpr float shadowWidth = 50.0f;
        constexpr float shadowHeight = 50.0f;
        constexpr float shadowDepth = 100.0f;

        // シャドウマップの基準となる（可視領域の中心の方が良いかもしれない）
        XMFLOAT3 center = view.eyePosition;

        // ライト方向と（仮想的な）ライト位置を計算してビュー行列を作成
        XMFLOAT3 dir = MiMath::Normalize(m_lightDirection);
        XMFLOAT3 eye = MiMath::Add(
            center, 
            MiMath::Multiply(dir, -shadowDepth * 0.5f));
        XMMATRIX viewMatrix = XMMatrixLookToLH(
            XMLoadFloat3(&eye),
            XMLoadFloat3(&dir),
            XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
        XMMATRIX projectionMatrix = XMMatrixOrthographicLH(
            shadowWidth, 
            shadowHeight, 
            0.1f, 
            shadowDepth);

        // ライトのビュー行列とプロジェクション行列をシェーダーに転送
        EngineServiceLocator::UpdateCameraCB({ 
            .view = viewMatrix, 
            .projection = projectionMatrix,
            .eyePos = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f)
            });
        m_shadowLightMatrix = viewMatrix * projectionMatrix;
    }

    //-------------------------------------
    // モデルの描画
    //-------------------------------------
    ModelResource::VertexType currentVertexType = ModelResource::VertexType::Static;
    EngineServiceLocator::BindShader(ShaderBase::Unlit);
    m_pContext->PSSetShader(nullptr, nullptr, 0);

    ModelRenderUtility::ForEachRenderableModel(
        pScene,
        [this, &currentVertexType](ModelComponent& m, TransformComponent& t, ModelResource& model)
        {
            // シェーダーを切り替え
            if (currentVertexType != model.vertexType) {
                switch (model.vertexType) {
                case ModelResource::VertexType::Static:
                    EngineServiceLocator::BindShader(ShaderBase::Unlit);
                    break;
                case ModelResource::VertexType::Skinned:
                    EngineServiceLocator::BindShader(ShaderBase::SkinnedUnlit);
                    break;
                }
                m_pContext->PSSetShader(nullptr, nullptr, 0);
                currentVertexType = model.vertexType;
            }

            // ワールド行列を計算
            XMMATRIX worldMatrix = ModelRenderUtility::CreateWorldMatrix(t);
            EngineServiceLocator::UpdateTransformCB({ worldMatrix, XMMatrixIdentity() });

            // スキニング行列を転送
            if (model.vertexType == ModelResource::VertexType::Skinned) {
                EngineServiceLocator::ModelRepository()->BindSkinningCB(m.GetSkeletonPose().boneTransforms);
            }

            // ジオメトリの描画
            m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            ModelRenderUtility::DrawMeshListGeometry(m_pContext, model.meshes);
        });

    //-------------------------------------
    // スプライトの描画
    //-------------------------------------
    EngineServiceLocator::BindShader(ShaderBase::SpriteUnlit);

    SpriteRenderUtility::ForEachRenderableSprite(
        pScene,
        [this](SpriteRendererComponent& s, TransformComponent& t)
        {
            if (s.GetBlendMode() == SpriteRendererComponent::SpriteBlendMode::AlphaBlend) return;
            if (s.GetBlendMode() == SpriteRendererComponent::SpriteBlendMode::Additive) return;

            // ライトから見て面が裏向きかどうか
            bool isBackFace = MiMath::Dot(t.GetForward(), m_lightDirection) < 0.0f;

            // ワールド行列を計算
            XMMATRIX worldMatrix = SpriteRenderUtility::CreateWorldMatrix(t);
            if (isBackFace) {
                // 裏面の場合は180度回転させる
                XMMATRIX flipRotation = XMMatrixRotationY(XMConvertToRadians(180.0f));
                worldMatrix = flipRotation * worldMatrix;
            }
            EngineServiceLocator::UpdateTransformCB({ worldMatrix, XMMatrixIdentity() });

            // テクスチャをバインド
            TextureResource* texture = s.GetTextureResource() ? s.GetTextureResource() : m_defaultTexture;
            if (texture) {
                m_pContext->PSSetShaderResources(0, 1, texture->texture.GetAddressOf());
            }

            // UV矩形にフリップを適用して頂点バッファを更新
            XMFLOAT4 uvRect = SpriteRenderUtility::ApplyFlipToUvRect(s.GetUvRect(), s.GetFlipX(), s.GetFlipY());
            if (isBackFace) {
                // 裏面の場合はUVも反転させる
                uvRect = SpriteRenderUtility::ApplyFlipToUvRect(uvRect, true, false);
            }
            if (!SpriteRenderUtility::UpdateSpriteVertexBuffer(m_pContext, m_spriteVertexBuffer.Get(), uvRect, s.GetColor())) {
                return;
            }

            ID3D11Buffer* vertexBuffer = m_spriteVertexBuffer.Get();
            UINT stride = sizeof(ShaderDefinitions::SpriteVertex);
            UINT offset = 0;
            m_pContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

            // 描画
            m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
            m_pContext->Draw(4, 0);
        });

    SetSamplerState(SAMPLERSTATE_POINT_WRAP);
}

// ----------------------------------- Bind

// シャドウマップ用のライト定数バッファをシェーダーにバインド
void ShadowMapPass::BindShadowCB()
{
    XMMATRIX transposedMatrix = XMMatrixTranspose(m_shadowLightMatrix);
    m_pContext->UpdateSubresource(m_shadowLightCB->buffer.Get(), 0, nullptr, &transposedMatrix, 0, 0);
}

// シャドウマップ用の深度テクスチャをピクセルシェーダーにバインド
void ShadowMapPass::BindShadowTexture()
{
    m_pContext->PSSetShaderResources(10, 1, depthBufferSRV.GetAddressOf());
}

// シャドウマップ用の比較サンプラーをピクセルシェーダーのs1にバインド
void ShadowMapPass::BindShadowSampler()
{
    ID3D11SamplerState* sampler = m_shadowComparisonSampler.Get();
    m_pContext->PSSetSamplers(1, 1, &sampler);
}

// シャドウマップ用の深度テクスチャをピクセルシェーダーから外す
void ShadowMapPass::UnbindShadowTexture()
{
    ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
    m_pContext->PSSetShaderResources(10, 1, nullSRV);
}

// シャドウマップ用の比較サンプラーをピクセルシェーダーのs1から外す
void ShadowMapPass::UnbindShadowSampler()
{
    ID3D11SamplerState* nullSampler[1] = { nullptr };
    m_pContext->PSSetSamplers(1, 1, nullSampler);
}
