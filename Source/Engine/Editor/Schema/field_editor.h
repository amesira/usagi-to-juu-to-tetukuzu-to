//---------------------------------------------------
// File  ：Engine/Editor/Schema/field_editor.h
// Date  ：2026/07/23
// Author：Miu Kitamura
// 
// ・FieldをEditor上で描画するための関数群
//---------------------------------------------------
#pragma once
#include "Engine/Asset/Schema/field.h"
#include "Engine/Asset/Schema/field_schema.h"
#include "Engine/Asset/Schema/field_options.h"
#include "Engine/Asset/Schema/enum_field_options.h"
#include "Engine/Asset/Schema/field_struct.h"

#include <DirectXMath.h>
#include "Engine/Editor/i_editor_window.h"

namespace FieldEditor
{
    using namespace DirectX;

    /// @brief FieldEditorのデフォルト実装（これを参照する場合はコンパイルエラーになる）
    template<class TValue, class TOptions>
    class FieldEditor;

    // === DrawFieldsの前方宣言 ===
    template<class TObject, class TSchema>
    bool DrawFields(TObject& object, const TSchema& schema);

#pragma region DrawFieldsのEntry関数
    /// @brief Fieldの値をEditor上で描画する関数
    /// @tparam TObject 対象となるオブジェクトの型（例：ParticleSystem、DataAssetなど）
    /// @tparam TValue 対象となるメンバ変数の型
    /// @tparam TOptions フィールドのオプションの型（例：範囲、デフォルト値など）
    template<class TObject, class TValue, class TOptions>
    bool DrawField(TObject& object, const Field<TObject, TValue, TOptions>& field)
    {
        TValue& value = object.*(field.member);
        return FieldEditor<TValue, TOptions>::Draw(field.label.c_str(), value, field.options);
    }

    /// @brief StructFieldの値をEditor上で描画する関数
    template<class TObject, class TStructValue, class TOptions, class... TFields>
    bool DrawField(TObject& object, const StructField<TObject, TStructValue, TOptions, TFields...>& field)
    {
        // StructFieldのLabelをTreeNodeとして表示し、展開可能にする
        if (!ImGui::TreeNode(field.label.c_str())) return false;

        TStructValue& structValue = object.*(field.member);
        bool changed = DrawFields(structValue, field.schema);

        ImGui::TreePop();
        return changed;
    }
#pragma endregion

    /// @brief FieldSchemaの各フィールドをEditor上で描画する関数
    /// @tparam TObject 
    /// @tparam TSchema 
    template<class TObject, class TSchema>
    bool DrawFields(TObject& object, const TSchema& schema)
    {
        bool changed = false;

        ImGui::PushID(static_cast<const void*>(&object));
        schema.ForEach([&](const auto& field) {
            ImGui::PushID(field.key.c_str());
            changed |= DrawField(object, field);
            ImGui::PopID();
        });
        ImGui::PopID();

        return changed;
    }

#pragma region FieldEditor DragFieldの特殊化
    /// @brief FieldEditorのfloat型の特殊化
    template<>
    class FieldEditor<float, DragFieldOptions> {
    public:
        static bool Draw(const char* label, float& value, const DragFieldOptions& options)
        {
            return ImGui::DragFloat(label, &value, options.dragSpeed, options.minValue, options.maxValue);
        }
    };
    
    /// @brief FieldEditorのint型の特殊化
    template<>
    class FieldEditor<int, DragFieldOptions> {
    public:
        static bool Draw(const char* label, int& value, const DragFieldOptions& options)
        {
            return ImGui::DragInt(
                label,
                &value,
                options.dragSpeed,
                static_cast<int>(options.minValue),
                static_cast<int>(options.maxValue));
        }
    };

    /// @brief FieldEditorのXMFLOAT2型の特殊化
    template<>
    class FieldEditor<XMFLOAT2, DragFieldOptions> {
    public:
        static bool Draw(const char* label, XMFLOAT2& value, const DragFieldOptions& options)
        {
            return ImGui::DragFloat2(label, &value.x, options.dragSpeed, options.minValue, options.maxValue);
        }
    };

    /// @brief FieldEditorのXMFLOAT3型の特殊化
    template<>
    class FieldEditor<XMFLOAT3, DragFieldOptions> {
    public:
        static bool Draw(const char* label, XMFLOAT3& value, const DragFieldOptions& options)
        {
            return ImGui::DragFloat3(label, &value.x, options.dragSpeed, options.minValue, options.maxValue);
        }
    };

    /// @brief FieldEditorのXMFLOAT4型の特殊化
    template<>
    class FieldEditor<XMFLOAT4, DragFieldOptions> {
    public:
        static bool Draw(const char* label, XMFLOAT4& value, const DragFieldOptions& options)
        {
            return ImGui::DragFloat4(label, &value.x, options.dragSpeed, options.minValue, options.maxValue);
        }
    };
#pragma endregion

#pragma region FieldEditor SliderFieldの特殊化
    /// @brief FieldEditorのfloat型のSliderFieldOptions特殊化
    template<>
    class FieldEditor<float, SliderFieldOptions> {
    public:
        static bool Draw(const char* label, float& value, const SliderFieldOptions& options)
        {
            return ImGui::SliderFloat(label, &value, options.minValue, options.maxValue);
        }
    };

    /// @brief FieldEditorのint型のSliderFieldOptions特殊化
    template<>
    class FieldEditor<int, SliderFieldOptions> {
    public:
        static bool Draw(const char* label, int& value, const SliderFieldOptions& options)
        {
            return ImGui::SliderInt(
                label,
                &value,
                static_cast<int>(options.minValue),
                static_cast<int>(options.maxValue));
        }
    };
#pragma endregion

    /// @brief FieldEditorのstd::string型の特殊化
    template<>
    class FieldEditor<std::string, DefaultFieldOptions> {
    public:
        static bool Draw(const char* label, std::string& value, const DefaultFieldOptions& options)
        {
            char buffer[512] = {};
            strncpy_s(buffer, value.c_str(), _TRUNCATE);
            if (!ImGui::InputText(label, buffer, sizeof(buffer))) return false;
            value = buffer;
            return true;
        }
    };

    /// @brief FieldEditorのbool型の特殊化
    template<>
    class FieldEditor<bool, DefaultFieldOptions> {
    public:
        static bool Draw(const char* label, bool& value, const DefaultFieldOptions& options)
        {
            return ImGui::Checkbox(label, &value);
        }
    };

#pragma region FieldEditor ColorFieldの特殊化
    /// @brief FieldEditorのXMFLOAT3型のColorFieldOptions特殊化
    /// TODO: HDRが未対応なので、直したい
    template<>
    class FieldEditor<XMFLOAT3, ColorFieldOptions> {
    public:
        static bool Draw(const char* label, XMFLOAT3& value, const ColorFieldOptions& options)
        {
            return ImGui::ColorEdit3(label, &value.x, options.useAlpha ? 0 : ImGuiColorEditFlags_NoAlpha);
        }
    };

    /// @brief FieldEditorのXMFLOAT4型のColorFieldOptions特殊化
    template<>
    class FieldEditor<XMFLOAT4, ColorFieldOptions> {
    public:
        static bool Draw(const char* label, XMFLOAT4& value, const ColorFieldOptions& options)
        {
            return ImGui::ColorEdit4(label, &value.x, options.useAlpha ? 0 : ImGuiColorEditFlags_NoAlpha);
        }
    };
#pragma endregion

#pragma region FieldEditor CurveFieldの特殊化
    /// @brief FieldEditorのFloatCurve型の特殊化
    template<>
    class FieldEditor<MiCurve::FloatCurve, DefaultFieldOptions> {
    public:
        static bool Draw(const char* label, MiCurve::FloatCurve& curve, const DefaultFieldOptions& options)
        {
            if (!ImGui::TreeNode(label)) return false;

            bool changed = false;

            // 各キーの時間と値を表示し、編集可能にする
            for (int index = 0; index < static_cast<int>(curve.keys.size()); index++)
            {
                ImGui::PushID(index);
                changed |= ImGui::DragFloat("Time", &curve.keys[index].time, 0.01f, 0.0f, 1.0f);
                changed |= ImGui::DragFloat("Value", &curve.keys[index].value, 0.01f);

                // Removeボタンを押すと、該当のキーを削除する
                if (ImGui::Button("Remove")) {
                    curve.keys.erase(curve.keys.begin() + index);
                    changed = true;
                    ImGui::PopID();
                    break;
                }
                ImGui::Separator();
                ImGui::PopID();
            }

            // Add Keyボタンを押すと、デフォルトのキーを追加する
            if (ImGui::Button("Add Key")) {
                curve.keys.push_back({ 1.0f, 1.0f });
                changed = true;
            }

            // キーの時間を昇順にソートする
            if (changed) {
                std::sort(curve.keys.begin(), curve.keys.end(), [](const auto& left, const auto& right) {
                    return left.time < right.time;
                    });
            }

            ImGui::TreePop();
            return changed;
        }
    };
#pragma endregion

    /// @brief FieldEditorのAngleFieldOptions特殊化
    template<>
    class FieldEditor<float, AngleFieldOptions> {
    public:
        static bool Draw(const char* label, float& value, const AngleFieldOptions& options)
        {
            return ImGui::SliderAngle(label, &value, options.minValue, options.maxValue);
        }
    };
}
