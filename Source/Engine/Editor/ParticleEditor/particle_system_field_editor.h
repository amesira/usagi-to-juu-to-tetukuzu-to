//---------------------------------------------------
// File  ：Engine/Editor/ParticleEditor/particle_system_field_editor.h
// Date  ：2026/07/26
// Author：Miu Kitamura
// 
// ・ParticleSystemData特有のFieldEditorの特殊化を定義する
//---------------------------------------------------
#pragma once
#include "Engine/Asset/ParticleAsset/particle_system_data.h"
#include "Engine/Editor/Schema/field_editor.h"

namespace FieldEditor
{
    template<>
    class FieldEditor<ParticleSystemData::MinMaxFloat, DefaultFieldOptions> {
    public:
        static bool Draw(const char* label, ParticleSystemData::MinMaxFloat& value, const DefaultFieldOptions& options)
        {
            if (!ImGui::TreeNode(label)) return false;

            bool changed = false;
            // ランダムかどうかのチェックボックス
            changed |= ImGui::Checkbox("Random Between Two Constants", &value.randomBetweenTwoConstants);
            if (value.randomBetweenTwoConstants) {
                changed |= ImGui::DragFloat("Constant Min", &value.constantMin, 0.1f);
                changed |= ImGui::DragFloat("Constant Max", &value.constantMax, 0.1f);
            }
            else {
                changed |= ImGui::DragFloat("Constant", &value.constant, 0.1f);
            }

            ImGui::TreePop();
            return changed;
        }
    };

    template<>
    class FieldEditor<ParticleSystemData::MinMaxColor, ColorFieldOptions> {
    public:
        static bool Draw(const char* label, ParticleSystemData::MinMaxColor& value, const DefaultFieldOptions& options)
        {
            if (!ImGui::TreeNode(label)) return false;

            bool changed = false;
            // ランダムかどうかのチェックボックス
            changed |= ImGui::Checkbox("Random Between Two Colors", &value.randomBetweenTwoColors);
            if (value.randomBetweenTwoColors) {
                changed |= ImGui::ColorEdit4("Color Min", reinterpret_cast<float*>(&value.colorMin));
                changed |= ImGui::ColorEdit4("Color Max", reinterpret_cast<float*>(&value.colorMax));
            }
            else {
                changed |= ImGui::ColorEdit4("Color", reinterpret_cast<float*>(&value.color));
            }

            ImGui::TreePop();
            return changed;
        }
    };
}