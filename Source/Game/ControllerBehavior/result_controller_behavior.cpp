#include "result_controller_behavior.h"

const ResultControllerSettings::Data& ResultControllerBehavior::Settings() const
{
    static const ResultControllerSettings::Data defaults;
    return m_settings ? m_settings->GetData() : defaults;
}

void ResultControllerBehavior::Start()
{
}

void ResultControllerBehavior::Update()
{
}

void ResultControllerBehavior::DrawComponentInspector()
{
}
