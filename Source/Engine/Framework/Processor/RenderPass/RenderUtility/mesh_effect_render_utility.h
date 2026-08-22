#pragma once

//---------------------------------------------------
// mesh_effect_render_utility.h
// ・MeshEffect描画に関連する共通処理
//---------------------------------------------------
#ifndef MESH_EFFECT_RENDER_UTILITY_H
#define MESH_EFFECT_RENDER_UTILITY_H

#include <functional>

#include "Engine/Device/direct3d.h"
#include "Engine/Asset/MeshEffectAsset/mesh_effect_data.h"

class IScene;
class MeshEffectComponent;
class TransformComponent;
struct RenderView;

namespace MeshEffectRenderData {
    struct MeshEffectBuffer;
    struct MeshEffectEvaluatedState;
}

namespace MeshEffectRenderUtility {

    DirectX::XMMATRIX CreateBillboardRotation(
        MeshEffectData::BillboardMode billboardMode,
        const RenderView& view);

    DirectX::XMMATRIX CreateWorldMatrix(
        const TransformComponent& transform,
        const MeshEffectRenderData::MeshEffectEvaluatedState& evaluatedState,
        const DirectX::XMMATRIX& billboardRotation);

    bool UpdatePixelConstantBuffer(
        ID3D11DeviceContext* context,
        ID3D11Buffer* constantBuffer,
        const MeshEffectRenderData::MeshEffectBuffer& bufferData);

    void DrawGeometry(
        ID3D11DeviceContext* context,
        const MeshEffectComponent& meshEffect);

    void ForEachRenderableMeshEffect(
        IScene* scene,
        const std::function<void(MeshEffectComponent&, TransformComponent&)>& callback);

}

#endif // MESH_EFFECT_RENDER_UTILITY_H
