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
#include <utility>
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
    explicit Field(std::string key, std::string label, TValue TObject::* member, TOptions options = TOptions{})
        :key(std::move(key)), label(std::move(label)), member(member), options(std::move(options)) {
    }
};

/// @brief Fieldのインスタンスを作成するためのヘルパー関数
/// @param key 
/// @param label 
/// @param member 
/// @param options 
/// @return 
template<class TObject, class TValue, class TOptions = DefaultFieldOptions>
auto MakeField(
    std::string key,
    std::string label,
    TValue TObject::* member,
    TOptions options = TOptions{})
{
    return Field<TObject, TValue, TOptions>(
        std::move(key),
        std::move(label),
        member,
        std::move(options));
}
