//---------------------------------------------------
// File  ：_/Asset/Schema/field_header.h
// Date  ：2026/08/15
// Author：Miu Kitamura
// 
// ・描画時にヘッダーを表示するためのHeaderFieldクラス
//---------------------------------------------------
#pragma once
#include <string>
#include <utility>

class HeaderField {
public:
    std::string key = "this_is_header_field";
    // SerializeFieldでエラーが起きないようにするためのダミーのキー名

    std::string label;  // Editor上でのラベル名
    explicit HeaderField(std::string label)
        : label(std::move(label)) {
    }
};

inline auto MakeHeaderField(std::string label) {
    return HeaderField(std::move(label));
}