//===================================================
// collector_image.cpp
// 
// Author：Miu Kitamura
// Date  ：2025/11/18
//===================================================
#include "collector_image.h"
#include "ui_chromatic_echo_utility.h"
#include "image_fill_utility.h"
#include "Engine/Core/scene_interface.h"
#include "Engine/Core/game_object.h"

#include "Engine/Component/rect_transform_component.h"
#include "Engine/Component/image_component.h"
#include "Engine/Component/transform_component.h"

#include "Engine/engine_service_locator.h"
#define SHADER_REPOSITORY EngineServiceLocator::GetShaderRepository()

// 初期化
void CollectorImage::Initialize()
{
    m_pDefaultUiShader = SHADER_REPOSITORY->GetShaderProgramResource(ShaderBase::Ui);
}

// 終了処理
void CollectorImage::Finalize()
{

}

// 2D描画用のDrawBatch2Dコマンドを収集
void CollectorImage::CollectDrawBatches2D(IScene* pScene, std::vector<UiDrawCommand::DrawBatch2D>& outBatches)
{
    auto* imagePool = pScene->GetComponentPool<ImageComponent>();
    auto* rectTransformPool = pScene->GetComponentPool<RectTransformComponent>();
    if(imagePool == nullptr || rectTransformPool == nullptr)return;

    auto& imageList = imagePool->GetList();

    for (auto& imageComp : imageList) {
        auto* image = &imageComp;
        auto* rect = rectTransformPool->GetByGameObjectID(image->GetOwner()->GetID());

        if (!rect) continue;
        if (!rect->GetEnable()) continue;
        if (!image->GetTextureResource()) continue;
        if (!image->GetEnable()) continue;
        if (!image->GetOwner()->GetActive()) continue;

        // 描画コマンドに追加
        UiDrawCommand::DrawBatch2D batch;
        batch.orderInLayer = rect->GetPosition().z;
        batch.texture = image->GetTextureResource()->texture.Get();
        batch.shaderProgram = m_pDefaultUiShader;

        UiDrawCommand::DrawCommand2DInstance instance;
        instance.position = { rect->GetPosition().x, rect->GetPosition().y };
        instance.size = { rect->GetScaling().x, rect->GetScaling().y };
        instance.angleZ = rect->GetRotation().z;
        instance.presentationTransform = rect->GetPresentationTransform();
        instance.color = image->GetColor();
        instance.uvRect = image->GetUvRect();
        if (!ImageFillUtility::Apply(*image, instance)) continue;

        batch.instances.push_back(instance);

        UiChromaticEchoUtility::Append(outBatches, std::move(batch), rect->GetChromaticEcho());
    }
}

void CollectorImage::CollectDrawBatches3D(IScene* pScene, std::vector<UiDrawCommand::DrawBatch3D>& outBatches)
{
    auto* images = pScene->GetComponentPool<ImageComponent>();
    auto* transforms = pScene->GetComponentPool<TransformComponent>();
    if (!images || !transforms) return;

    for (auto& image : images->GetList()) {
        auto* owner = image.GetOwner();
        if (!owner || !owner->GetActive() || !image.GetEnable()) continue;
        auto* transform = transforms->GetByGameObjectID(owner->GetID());
        auto* texture = image.GetTextureResource();
        if (!transform || !transform->GetEnable() || !texture || !texture->texture) continue;

        UiDrawCommand::DrawCommand3DInstance instance;
        instance.position = transform->GetPosition();
        instance.scale = transform->GetScaling();
        instance.color = image.GetColor();
        instance.uvRect = image.GetUvRect();
        if (!ImageFillUtility::Apply(image, instance)) continue;
        UiDrawCommand::FindOrAddBatch3D(outBatches, texture->texture.Get(), m_pDefaultUiShader)
            .instances.push_back(instance);
    }
}
