//---------------------------------------------------
// File  ：Engine/Asset/Schema/field_schema.h
// Date  ：2026/07/23
// Author：Miu Kitamura
// 
// ・複数のFieldをまとめて管理するためのクラス
//---------------------------------------------------
#pragma once
#include <functional>
#include <tuple>
#include <utility>

#include "field.h"

template<class... TFields> // MEMO: 0個以上の型を受け取る可変長テンプレート
class FieldSchema {
private:
    std::tuple<TFields...> m_fields;
    // MEMO: std::tupleとTFields...を使うことで、異なる型のFieldをまとめて管理できる
    // （例：Field<ParticleSystem, float, RangeOption>とField<ParticleSystem, int, DefaultOption>を同時に管理できる）

public:
    explicit FieldSchema(TFields... fields) : m_fields(std::move(fields)...) {
    }

    /// @brief Fieldのタプルを取得する
    std::tuple<TFields...>& GetFields() {
        return m_fields;
    }

    /// @brief FieldのForEach処理を行う
    /// @tparam TFunction 関数オブジェクトの型
    /// @param function 関数オブジェクト
    template<class TFunction>
    void ForEach(TFunction&& function) 
    {
        std::apply(
            [&](auto&... fields) {
                (
                    std::invoke(function, fields),
                    ...
                    );
            },
            m_fields);
    }

    /// @brief FieldのForEach処理を行う（const版）
    template<class TFunction>
    void ForEach(TFunction&& function) const 
    {
        std::apply(
            [&](const auto&... fields) {
                (
                    std::invoke(function,fields),
                    ...
                );
            },
            m_fields);
    }
};