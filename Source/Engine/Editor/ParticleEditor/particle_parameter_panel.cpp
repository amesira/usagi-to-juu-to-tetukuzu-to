#include "particle_parameter_panel.h"

#include <algorithm>
#include <cstring>

#include "External/ImGui/imgui.h"
#include "Engine/Asset/particle_system_schema.h"
#include "Engine/Editor/Schema/enum_field_editor.h"
#include "Engine/Editor/Schema/field_editor.h"

namespace
{
    bool InputTextString(const char* label, std::string& value)
    {
        char buffer[512] = {};
        strncpy_s(buffer, value.c_str(), _TRUNCATE);
        if (!ImGui::InputText(label, buffer, sizeof(buffer))) return false;
        value = buffer;
        return true;
    }
}

bool ParticleParameterPanel::Draw(ParticleSystemDesc& desc)
{
    bool changed = false;
    changed |= DrawMain(desc.mainModule);
    changed |= DrawEmission(desc.emissionModule);
    changed |= DrawShape(desc.shapeModule);
    changed |= DrawSizeOverLifetime(desc.sizeOverLifetimeModule);
    changed |= DrawTextureSheetAnimation(desc.textureSheetAnimation);
    changed |= DrawRenderer(desc.rendererModule);
    return changed;
}

bool ParticleParameterPanel::DrawMain(ParticleSystemData::MainModule& module)
{
    if (!ImGui::CollapsingHeader("Main", ImGuiTreeNodeFlags_DefaultOpen)) return false;
    bool changed = false;
    changed |= ImGui::DragFloat("Duration", &module.duration, 0.01f, 0.0f);
    changed |= ImGui::Checkbox("Loop", &module.loop);
    changed |= ImGui::Checkbox("Play On Awake", &module.playOnAwake);
    changed |= DrawMinMaxFloat("Start Lifetime", module.startLifetime);
    changed |= DrawMinMaxFloat("Start Speed", module.startSpeed);
    changed |= DrawMinMaxFloat("Start Size", module.startSize);
    changed |= DrawMinMaxColor("Start Color", module.startColor);
    changed |= ImGui::DragFloat3("Gravity", &module.gravity.x, 0.01f);
    changed |= ImGui::DragFloat("Simulation Speed", &module.simulationSpeed, 0.01f, 0.0f);

    int simulationSpace = static_cast<int>(module.simulationSpace);
    const char* items[] = { "Local", "World" };
    if (ImGui::Combo("Simulation Space", &simulationSpace, items, IM_ARRAYSIZE(items)))
    {
        module.simulationSpace = static_cast<ParticleSystemData::SimulationSpace>(simulationSpace);
        changed = true;
    }
    return changed;
}

bool ParticleParameterPanel::DrawEmission(ParticleSystemData::EmissionModule& module)
{
    if (!ImGui::CollapsingHeader("Emission", ImGuiTreeNodeFlags_DefaultOpen)) return false;
    return FieldEditor::DrawFields(
        module,
        ParticleSystemSchema::GetEmissionSchema());
}

bool ParticleParameterPanel::DrawShape(ParticleSystemData::ShapeModule& module)
{
    if (!ImGui::CollapsingHeader("Shape", ImGuiTreeNodeFlags_DefaultOpen)) return false;
    bool changed = false;
    changed |= ImGui::Checkbox("Enabled##Shape", &module.enabled);
    int shapeType = static_cast<int>(module.type);
    const char* items[] = { "Sphere", "Cone" };
    if (ImGui::Combo("Shape Type", &shapeType, items, IM_ARRAYSIZE(items)))
    {
        module.type = static_cast<ParticleSystemData::ShapeType>(shapeType);
        changed = true;
    }

    if (module.type == ParticleSystemData::ShapeType::Sphere)
    {
        changed |= ImGui::DragFloat("Radius##Sphere", &module.sphere.radius, 0.01f, 0.0f);
        changed |= ImGui::Checkbox("Emit From Shell", &module.sphere.emitFromShell);
    }
    else
    {
        float degrees = DirectX::XMConvertToDegrees(module.cone.angle);
        if (ImGui::DragFloat("Angle", &degrees, 0.1f, 0.0f, 180.0f))
        {
            module.cone.angle = DirectX::XMConvertToRadians(degrees);
            changed = true;
        }
        changed |= ImGui::DragFloat("Radius##Cone", &module.cone.radius, 0.01f, 0.0f);
        changed |= ImGui::DragFloat("Length", &module.cone.length, 0.01f, 0.0f);
        changed |= ImGui::Checkbox("Emit From Base", &module.cone.emitFromBase);
    }
    changed |= ImGui::DragFloat("Random Direction", &module.randomDirectionAmount, 0.01f, 0.0f, 1.0f);
    return changed;
}

bool ParticleParameterPanel::DrawSizeOverLifetime(ParticleSystemData::SizeOverLifetimeModule& module)
{
    if (!ImGui::CollapsingHeader("Size Over Lifetime")) return false;
    bool changed = ImGui::Checkbox("Enabled##SizeOverLifetime", &module.enabled);
    changed |= DrawFloatCurve("Size Curve", module.size);
    return changed;
}

bool ParticleParameterPanel::DrawTextureSheetAnimation(ParticleSystemData::TextureSheetAnimation& module)
{
    if (!ImGui::CollapsingHeader("Texture Sheet Animation")) return false;
    return FieldEditor::DrawFields(
        module,
        ParticleSystemSchema::GetTextureSheetAnimationSchema());
}

bool ParticleParameterPanel::DrawRenderer(ParticleSystemData::RendererModule& module)
{
    if (!ImGui::CollapsingHeader("Renderer", ImGuiTreeNodeFlags_DefaultOpen)) return false;
    bool changed = InputTextString("Texture Path", module.texturePath);
    changed |= ImGui::DragFloat4("UV Rect", &module.uvRect.x, 0.01f);

    int billboardMode = static_cast<int>(module.billboardMode);
    const char* billboardItems[] = { "View", "Horizontal" };
    if (ImGui::Combo("Billboard Mode", &billboardMode, billboardItems, IM_ARRAYSIZE(billboardItems)))
    {
        module.billboardMode = static_cast<ParticleSystemData::BillboardMode>(billboardMode);
        changed = true;
    }

    int blendMode = static_cast<int>(module.blendMode);
    const char* blendItems[] = { "Alpha Blend", "Additive" };
    if (ImGui::Combo("Blend Mode", &blendMode, blendItems, IM_ARRAYSIZE(blendItems)))
    {
        module.blendMode = static_cast<ParticleSystemData::BlendMode>(blendMode);
        changed = true;
    }
    changed |= ImGui::Checkbox("Sort By Distance", &module.sortByDistance);
    return changed;
}

bool ParticleParameterPanel::DrawMinMaxFloat(const char* label, ParticleSystemData::MinMaxFloat& value)
{
    if (!ImGui::TreeNode(label)) return false;
    bool changed = ImGui::Checkbox("Random Between Two Constants", &value.randomBetweenTwoConstants);
    if (value.randomBetweenTwoConstants)
    {
        changed |= ImGui::DragFloat("Min", &value.constantMin, 0.01f);
        changed |= ImGui::DragFloat("Max", &value.constantMax, 0.01f);
    }
    else
    {
        changed |= ImGui::DragFloat("Constant", &value.constant, 0.01f);
    }
    ImGui::TreePop();
    return changed;
}

bool ParticleParameterPanel::DrawMinMaxColor(const char* label, ParticleSystemData::MinMaxColor& value)
{
    if (!ImGui::TreeNode(label)) return false;
    bool changed = ImGui::Checkbox("Random Between Two Colors", &value.randomBetweenTwoColors);
    if (value.randomBetweenTwoColors)
    {
        changed |= ImGui::ColorEdit4("Min", &value.colorMin.x);
        changed |= ImGui::ColorEdit4("Max", &value.colorMax.x);
    }
    else
    {
        changed |= ImGui::ColorEdit4("Color", &value.color.x);
    }
    ImGui::TreePop();
    return changed;
}

bool ParticleParameterPanel::DrawFloatCurve(const char* label, MiCurve::FloatCurve& curve)
{
    if (!ImGui::TreeNode(label)) return false;
    bool changed = false;
    for (int index = 0; index < static_cast<int>(curve.keys.size()); ++index)
    {
        ImGui::PushID(index);
        changed |= ImGui::DragFloat("Time", &curve.keys[index].time, 0.01f, 0.0f, 1.0f);
        changed |= ImGui::DragFloat("Value", &curve.keys[index].value, 0.01f);
        if (ImGui::Button("Remove"))
        {
            curve.keys.erase(curve.keys.begin() + index);
            changed = true;
            ImGui::PopID();
            break;
        }
        ImGui::Separator();
        ImGui::PopID();
    }
    if (ImGui::Button("Add Key"))
    {
        curve.keys.push_back({ 1.0f, 1.0f });
        changed = true;
    }
    if (changed)
    {
        std::sort(curve.keys.begin(), curve.keys.end(), [](const auto& left, const auto& right) {
            return left.time < right.time;
        });
    }
    ImGui::TreePop();
    return changed;
}
