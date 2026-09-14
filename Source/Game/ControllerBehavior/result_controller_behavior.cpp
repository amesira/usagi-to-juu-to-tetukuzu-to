#include "result_controller_behavior.h"

#include "Engine/Core/game_object.h"
#include "Engine/Core/scene_interface.h"

#include "Engine/Component/transform_component.h"

#include "Game/Factory/prefab_factory.h"

const ResultControllerSettings::Data& ResultControllerBehavior::Settings() const
{
    static const ResultControllerSettings::Data defaults;
    return m_settings ? m_settings->GetData() : defaults;
}

void ResultControllerBehavior::Start()
{
    // ResultTextObjectを生成して配置
    GameObject* resultTextObject = PrefabFactory::CreateModelObject(
        GetOwner()->GetScene(),
        "asset/Model/result_logo.fbx",
        Settings().resultTextObject.position,
        Settings().resultTextObject.rotation,
        Settings().resultTextObject.scale);
    resultTextObject->SetName("ResultTextObject");
    resultTextObject->SetRenderLayer(RenderLayer::Particle);
    m_resultTextTransform = resultTextObject ? resultTextObject->GetComponent<TransformComponent>() : nullptr;

    m_settingsAssetRivision = m_settings ? m_settings->GetRevision() : -1;
}

void ResultControllerBehavior::Update()
{
    if (m_settings && m_settings->GetRevision() != m_settingsAssetRivision) {
        m_settingsAssetRivision = m_settings->GetRevision();

        m_resultTextTransform->SetPosition(Settings().resultTextObject.position);
        m_resultTextTransform->SetEulerRawAngle(Settings().resultTextObject.rotation);
        m_resultTextTransform->SetScaling(Settings().resultTextObject.scale);
    }
}

void ResultControllerBehavior::DrawComponentInspector()
{

}
