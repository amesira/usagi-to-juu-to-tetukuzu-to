//---------------------------------------------------
// File  ：Engine/Asset/Schema/field.h
// Date  ：2026/07/22
// Author：Miu Kitamura
// 
// ・1つのフィールドについての情報を表すクラス
// ・ParticleSystemやDataAssetなどのスキーマで使用される
//---------------------------------------------------
#pragma once
#include <string>
#include "field_options.h"

/// @brief 1つのフィールドについての情報を表す構造体
/// @tparam TObject 対象となるオブジェクトの型（例：ParticleSystem、DataAssetなど）
/// @tparam TValue 対象となるメンバ変数の型
/// @tparam TOptions フィールドのオプションの型（例：範囲、デフォルト値など）
template<class TObject, class TValue, class TOptions>
class Field {
public:
    std::string key;    // JSON上でのキー名
    std::string label;  // Editor上でのラベル名

    TValue TObject::* member;   // メンバ変数へのポインタ
    TOptions options;           // フィールドのオプション（enum変換用の文字列、範囲、デフォルト値など）

    // TOptionが{}の場合、型ごとにデフォルトのオプションを設定する
    explicit Field(std::string key, std::string label, TValue TObject::* member, TOptions options = DefaultFieldOptions())
        :key(key), label(label), member(member), options(options) {
    }
};