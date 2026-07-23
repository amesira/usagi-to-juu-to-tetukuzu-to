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
template<class T>
class DragFieldOptions {
public:
    float dragSpeed = 0.1f;  // ドラッグ速度
    T minValue = T{};  // 最小値
    T maxValue = T{};  // 最大値
    // MEMO: minValueとmaxValueがT{}の場合、制限なしとみなす
};

/// @brief Editorで使用するSliderFieldのオプション
template<class T>
class SliderFieldOptions {
public:
    T minValue = static_cast<T>(0.0f);  // 最小値
    T maxValue = static_cast<T>(1.0f);  // 最大値
};

/// @brief Editorで使用するColorFieldのオプション
class ColorFieldOptions {
public:
    bool useAlpha = true; // アルファ値を使用するかどうか
    bool useHDR = false; // HDRカラーを使用するかどうか
};

// Curve
// Angle
// Pathなど
