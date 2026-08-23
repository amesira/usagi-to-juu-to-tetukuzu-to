#include "mesh_effect_render_utility.h"

#include <cmath>

#include "Engine/Core/game_object.h"
#include "Engine/Core/scene_interface.h"
#include "Engine/Component/mesh_effect_component.h"
#include "Engine/Component/transform_component.h"
#include "Engine/Processor/RenderPass/RenderUtility/model_render_utility.h"
#include "Engine/render_view.h"

using namespace DirectX;

namespace MeshEffectRenderUtility {
    /// @brief ビルボード回転行列を作成する
    DirectX::XMMATRIX CreateBillboardRotation(
        MeshEffectData::BillboardMode billboardMode,
        const RenderView& view)
    {
        switch (billboardMode) {
        case MeshEffectData::BillboardMode::View: {
            XMMATRIX billboard = XMMatrixInverse(nullptr, view.viewMatrix);
            billboard.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
            return XMMatrixRotationY(XM_PI) * billboard;
        }
        case MeshEffectData::BillboardMode::Horizontal: {
            const XMMATRIX invView = XMMatrixInverse(nullptr, view.viewMatrix);

            XMFLOAT3 cameraForward = {};
            XMStoreFloat3(&cameraForward, invView.r[2]);
            cameraForward.y = 0.0f;

            const float lengthSq =
                cameraForward.x * cameraForward.x +
                cameraForward.z * cameraForward.z;
            if (lengthSq <= 0.0001f) return XMMatrixIdentity();

            const float yaw = std::atan2(cameraForward.x, cameraForward.z);
            return XMMatrixRotationY(XM_PI) * XMMatrixRotationY(yaw);
        }
        default:
            return XMMatrixIdentity();
        }
    }

    /// @brief ワールド行列を作成する
    DirectX::XMMATRIX CreateWorldMatrix(
        const TransformComponent& transform,
        const MeshEffectRenderData::MeshEffectEvaluatedState& evaluatedState,
        const DirectX::XMMATRIX& billboardRotation)
    {
        const XMFLOAT3 position = transform.GetPosition();
        const XMMATRIX translation = XMMatrixTranslation(position.x, position.y, position.z);

        // ComponentのTransformスケールは維持し、回転はBillboardの後に合成する。
        const XMFLOAT3 ownerScale = transform.GetScaling();
        const XMMATRIX ownerScaling = XMMatrixScaling(ownerScale.x, ownerScale.y, ownerScale.z);

        return evaluatedState.localEffectMatrix *
            ownerScaling *
            billboardRotation *
            translation;
    }

    /// @brief ピクセルシェーダー用の定数バッファを更新する
    bool UpdateCB(
        ID3D11DeviceContext* context,
        ID3D11Buffer* constantBuffer,
        const MeshEffectRenderData::MeshEffectBuffer& bufferData)
    {
        if (!context || !constantBuffer) return false;
        D3D11_MAPPED_SUBRESOURCE msr = {};
        context->Map(constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
        {
            MeshEffectRenderData::MeshEffectBuffer* cbData = (MeshEffectRenderData::MeshEffectBuffer*)msr.pData;
            *cbData = bufferData;
        }
        context->Unmap(constantBuffer, 0);
        return true;
    }

    /// @brief ジオメトリを描画する
    void DrawGeometry(
        ID3D11DeviceContext* context,
        const MeshEffectComponent& meshEffect)
    {
        if (!context) return;

        ModelResource* modelResource = meshEffect.GetModelResource();
        if (!modelResource) return;

        ModelRenderUtility::DrawMeshListGeometry(context, modelResource->meshes);
    }

    /// @brief シーン内の有効なMeshEffectComponentをすべて列挙し、コールバック関数を呼び出す
    void ForEachRenderableMeshEffect(
        IScene* scene,
        const std::function<void(MeshEffectComponent&, TransformComponent&)>& callback)
    {
        if (!scene || !callback) return;

        auto* transformPool = scene->GetComponentPool<TransformComponent>();
        auto* meshEffectPool = scene->GetComponentPool<MeshEffectComponent>();
        if (!transformPool || !meshEffectPool) return;

        for (MeshEffectComponent& meshEffect : meshEffectPool->GetList()) {
            TransformComponent* transform =
                transformPool->GetByGameObjectID(meshEffect.GetOwner()->GetID());
            if (!transform) continue;
            if (!meshEffect.GetOwner()->GetActive()) continue;
            if (!meshEffect.GetEnable() || !transform->GetEnable()) continue;
            if (!meshEffect.EvaluatedState().visible) continue;
            if (!meshEffect.GetModelResource()) continue;

            callback(meshEffect, *transform);
        }
    }

}
