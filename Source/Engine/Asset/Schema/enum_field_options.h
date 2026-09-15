//---------------------------------------------------
// File  ：Engine/Asset/Schema/enum_field_options.h
// Date  ：2026/07/23
// Author：Miu Kitamura
// 
// ・enum型のFieldOptionsの特殊化
//---------------------------------------------------
#pragma once
#include <string>
#include <vector>

/// @brief enum型フィールドの1要素を表す構造体
template<class TEnum>
class EnumChoice {
public:
    TEnum value;        // enumの値
    std::string key;    // JSON上でのキー名
    std::string label;  // Editor上でのラベル名
};

/// @brief enum型フィールドのオプションを表すクラス
template<class TEnum>
class EnumFieldOptions {
public:
    std::vector<EnumChoice<TEnum>> choices; // enumの選択肢のリスト
};