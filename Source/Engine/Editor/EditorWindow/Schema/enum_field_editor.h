//---------------------------------------------------
// File  ：Engine/Editor/EditorWindow/Schema/enum_field_editor.h
// Date  ：2026/07/23
// Author：Miu Kitamura
// 
// ・enum型のFieldEditorの特殊化
//---------------------------------------------------
#pragma once
#include "field_editor.h"

namespace FieldEditor
{
    /// @brief enum型のFieldEditorの特殊化
    template<class TEnum>
    class FieldEditor<TEnum, EnumFieldOptions<TEnum>> {
    public:
        static bool Draw(const char* label, TEnum& value, const EnumFieldOptions<TEnum>& options)
        {
            if (options.choices.empty()) {
                return false; // 選択肢がない場合は描画しない
            }

            bool changed = false;

            // 選択肢のラベルを取得する
            const char* previewLabel = "Unknown";
            for (const auto& choice : options.choices) {
                if (choice.value == value) {
                    previewLabel = choice.label.c_str();
                    break;
                }
            }

            // === ImGuiのコンボボックスを描画する ===
            if (ImGui::BeginCombo(label, previewLabel)) {
                for (const auto& choice : options.choices) {
                    // 選択されているか選択肢かどうかを判定する
                    const bool selected = choice.value == value;

                    // 選択が変更された場合は値を更新する
                    if (ImGui::Selectable(choice.label.c_str(), selected)) {
                        value = choice.value;
                        changed = true;
                    }

                    if (selected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }

                ImGui::EndCombo();
            }

            return changed;
        }
    };
}