//---------------------------------------------------
// File  ：Engine/Editor/EditorWindow/Schema/field_editor.h
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

#include <DirectXMath.h>
#include "Engine/Editor/i_editor_window.h"

namespace FieldEditor
{
    using namespace DirectX;

    /// @brief FieldEditorのデフォルト実装（これを参照する場合はコンパイルエラーになる）
    template<class TValue, class TOptions>
    class FieldEditor;

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

#pragma region FieldEditor DragFieldの特殊化
    /// @brief FieldEditorのfloat型の特殊化
    template<>
    class FieldEditor<float, DragFieldOptions<float>> {
    public:
        static bool Draw(const char* label, float& value, const DragFieldOptions<float>& options)
        {
            return ImGui::DragFloat(label, &value, options.dragSpeed, options.minValue, options.maxValue);
        }
    };
    
    /// @brief FieldEditorのint型の特殊化
    template<>
    class FieldEditor<int, DragFieldOptions<int>> {
    public:
        static bool Draw(const char* label, int& value, const DragFieldOptions<int>& options)
        {
            return ImGui::DragInt(label, &value, options.dragSpeed, options.minValue, options.maxValue);
        }
    };

    /// @brief FieldEditorのXMFLOAT2型の特殊化
    template<>
    class FieldEditor<XMFLOAT2, DragFieldOptions<XMFLOAT2>> {
    public:
        static bool Draw(const char* label, XMFLOAT2& value, const DragFieldOptions<XMFLOAT2>& options)
        {
            return ImGui::DragFloat2(label, &value.x, options.dragSpeed, options.minValue.x, options.maxValue.x);
        }
    };

    /// @brief FieldEditorのXMFLOAT3型の特殊化
    template<>
    class FieldEditor<XMFLOAT3, DragFieldOptions<XMFLOAT3>> {
    public:
        static bool Draw(const char* label, XMFLOAT3& value, const DragFieldOptions<XMFLOAT3>& options)
        {
            return ImGui::DragFloat3(label, &value.x, options.dragSpeed, options.minValue.x, options.maxValue.x);
        }
    };

    /// @brief FieldEditorのXMFLOAT4型の特殊化
    template<>
    class FieldEditor<XMFLOAT4, DragFieldOptions<XMFLOAT4>> {
    public:
        static bool Draw(const char* label, XMFLOAT4& value, const DragFieldOptions<XMFLOAT4>& options)
        {
            return ImGui::DragFloat4(label, &value.x, options.dragSpeed, options.minValue.x, options.maxValue.x);
        }
    };
#pragma endregion

#pragma region FieldEditor SliderFieldの特殊化
    /// @brief FieldEditorのfloat型のSliderFieldOptions特殊化
    template<>
    class FieldEditor<float, SliderFieldOptions<float>> {
    public:
        static bool Draw(const char* label, float& value, const SliderFieldOptions<float>& options)
        {
            return ImGui::SliderFloat(label, &value, options.minValue, options.maxValue);
        }
    };

    /// @brief FieldEditorのint型のSliderFieldOptions特殊化
    template<>
    class FieldEditor<int, SliderFieldOptions<int>> {
    public:
        static bool Draw(const char* label, int& value, const SliderFieldOptions<int>& options)
        {
            return ImGui::SliderInt(label, &value, options.minValue, options.maxValue);
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

}