//---------------------------------------------------
// File  ：Engine/Asset/Schema/field_struct.h
// Date  ：2026/07/26
// Author：Miu Kitamura
// 
// ・FieldSchemaをネストして管理するためのクラス
// ・構造体をメンバ変数として持つ場合に使用する
//---------------------------------------------------
#pragma once
#include <string>
#include "field_schema.h"

template<class TObject, class TStructValue, class TOptions, class... TFields>
class StructField {
public:
    std::string key;    // JSON上でのキー名
    std::string label;  // Editor上でのラベル名

    TStructValue TObject::* member;   // メンバ変数へのポインタ（指定の構造体）
    TOptions options;           // フィールドのオプション

    FieldSchema<TFields...> schema;

    explicit StructField(std::string key, std::string label, TStructValue TObject::* member, FieldSchema<TFields...> schema, TOptions options = TOptions{})
        : key(std::move(key)), label(std::move(label)), member(member), options(std::move(options)), schema(std::move(schema)) {
    }

};

/// @brief StructFieldのインスタンスを作成するためのヘルパー関数
/// @param key 
/// @param label 
/// @param member 
/// @param options 
/// @param schema 
/// @return 
template<class TObject, class TStructValue, class TOptions, class... TFields>
auto MakeStructField(
    std::string key,
    std::string label,
    TStructValue TObject::* member,
    FieldSchema<TFields...> schema,
    TOptions options = TOptions{})
{
    return StructField<TObject, TStructValue, TOptions, TFields...>(
        std::move(key),
        std::move(label),
        member,
        std::move(schema),
        std::move(options));
}