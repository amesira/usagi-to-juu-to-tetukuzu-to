//---------------------------------------------------
// File  ：Engine/Asset/Schema/field_options.h
// Date  ：2026/07/23
// Author：Miu Kitamura
// 
// ・FieldSerializer、FieldEditorで使用するオプションを定義する
//---------------------------------------------------
#pragma once

/// @brief デフォルトのFieldOptionsクラス
class DefaultFieldOptions {

};

/// @brief Editorで使用するDragFieldのオプション
class DragFieldOptions {
public:
    float dragSpeed = 0.1f;  // ドラッグ速度
    float minValue = 0.0f;   // 最小値
    float maxValue = 1.0f;   // 最大値
};

/// @brief Editorで使用するSliderFieldのオプション
class SliderFieldOptions {
public:
    float minValue = 0.0f;   // 最小値
    float maxValue = 1.0f;   // 最大値
};

/// @brief Editorで使用するColorFieldのオプション
class ColorFieldOptions {
public:
    bool useAlpha = true; // アルファ値を使用するかどうか
    bool useHDR = false; // HDRカラーを使用するかどうか
};

/// @brief Editorで使用するAngleFieldのオプション
class AngleFieldOptions {
public:
    float minValue = 0.0f;   // 最小値
    float maxValue = 360.0f; // 最大値
};