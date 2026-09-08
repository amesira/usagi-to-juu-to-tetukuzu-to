//===================================================
// collector_slider.cpp
// 
// Author：Miu Kitamura
// Date  ：2025/11/27
//===================================================
#include "collector_slider.h"
#include "Engine/Core/scene_interface.h"
#include "Engine/Core/game_object.h"

#include "Engine/Device/direct3d.h"

#include "Engine/Component/rect_transform_component.h"
#include "Engine/Component/slider_component.h"
#include "Engine/Component/transform_component.h"

#include "Engine/engine_service_locator.h"
#define TEXTURE_REPOSITORY EngineServiceLocator::GetTextureRepository()
#define SHADER_REPOSITORY EngineServiceLocator::GetShaderRepository()

void CollectorSlider::Initialize()
{
    m_pDefaultTexture = TEXTURE_REPOSITORY->GetTextureResource(L"asset/Texture/white.bmp");
    m_pDefaultUiShader = SHADER_REPOSITORY->GetShaderProgramResource(ShaderBase::Ui);
}

void CollectorSlider::Finalize()
{

}

void CollectorSlider::CollectDrawBatches2D(IScene* pScene, std::vector<UiDrawCommand::DrawBatch2D>& outBatches)
{
    auto* sliderComponentPool = pScene->GetComponentPool<SliderComponent>();
    auto* rectTransformPool = pScene->GetComponentPool<RectTransformComponent>();
    if (!sliderComponentPool || !rectTransformPool || !m_pDefaultTexture)return;

    auto& sliderList = sliderComponentPool->GetList();

    for (SliderComponent& sliderComp : sliderList)
    {
        SliderComponent* slider = &sliderComp;
        RectTransformComponent* rect = rectTransformPool->GetByGameObjectID(slider->GetOwner()->GetID());

        if (!rect) continue;
        if (!rect->GetEnable()) continue;
        if (!slider->GetOwner()->GetActive()) continue;
        if (!slider->GetEnable()) continue;

        // 描画コマンドに追加
        UiDrawCommand::DrawBatch2D batch;
        batch.orderInLayer = rect->GetPosition().z;
        batch.texture = m_pDefaultTexture->texture.Get();
        batch.shaderProgram = m_pDefaultUiShader;

        // BG描画コマンド
        UiDrawCommand::DrawCommand2DInstance instance;
        instance.position = { rect->GetPosition().x, rect->GetPosition().y };
        instance.size = { rect->GetScaling().x, rect->GetScaling().y };
        instance.angleZ = rect->GetRotation().z;
        instance.presentationTransform = rect->GetPresentationTransform();
        instance.color = slider->GetBgColor();
        instance.uvRect = {0.0f, 0.0f, 1.0f, 1.0f};

        batch.instances.push_back(instance);

        // Magicalでは、ここに遅延塗りつぶしを描画している

        // 塗りつぶし描画コマンド
        float value = slider->GetValue();
        {
            XMFLOAT2 fillLocalPos = { -(rect->GetScaling().x * (1.0f - value)) * 0.5f, 0.0f };
            float cosA = cosf(instance.angleZ);
            float sinA = sinf(instance.angleZ);
            XMFLOAT2 fillRotatedPos = {
                fillLocalPos.x * cosA - fillLocalPos.y * sinA,
                fillLocalPos.x * sinA + fillLocalPos.y * cosA
            };
            instance.position = {
                rect->GetPosition().x + fillRotatedPos.x,
                rect->GetPosition().y + fillRotatedPos.y
            };
        }
        instance.size.x = rect->GetScaling().x * value;
        instance.color = slider->GetFillColor();

        batch.instances.push_back(instance);

        outBatches.push_back(batch);
    }
}

void CollectorSlider::CollectDrawBatches3D(IScene* pScene, std::vector<UiDrawCommand::DrawBatch3D>& outBatches)
{
    auto* sliders = pScene->GetComponentPool<SliderComponent>();
    auto* transforms = pScene->GetComponentPool<TransformComponent>();
    if (!sliders || !transforms || !m_pDefaultTexture || !m_pDefaultTexture->texture) return;

    for (auto& slider : sliders->GetList()) {
        auto* owner = slider.GetOwner();
        if (!owner || !owner->GetActive() || !slider.GetEnable()) continue;
        auto* transform = transforms->GetByGameObjectID(owner->GetID());
        if (!transform || !transform->GetEnable()) continue;

        auto& batch = UiDrawCommand::FindOrAddBatch3D(outBatches,
            m_pDefaultTexture->texture.Get(), m_pDefaultUiShader);
        UiDrawCommand::DrawCommand3DInstance instance;
        instance.position = transform->GetPosition();
        instance.scale = transform->GetScaling();
        instance.color = slider.GetBgColor();
        batch.instances.push_back(instance);

        const float value = slider.GetValue();
        if (value <= 0.0f) continue;
        instance.offset.x = -instance.scale.x * (1.0f - value) * 0.5f;
        instance.scale.x *= value;
        instance.color = slider.GetFillColor();
        batch.instances.push_back(instance);
    }
}
