#pragma once
#include <filesystem>
#include <string>
#include <string_view>

namespace ModelTexturePath {
    // AssimpのパスはUTF-8。相対参照はモデルファイルの親フォルダ基準。
    inline std::filesystem::path ResolveExternal(const std::filesystem::path& modelPath, std::string_view reference) {
        if (reference.empty() || reference.front() == '*') return {}; // 埋め込みテクスチャは対象外
        const std::u8string utf8(reference.begin(), reference.end());
        const std::filesystem::path texturePath(utf8);
        if (texturePath.is_absolute()) return texturePath.lexically_normal();
        return (modelPath.parent_path() / texturePath).lexically_normal();
    }
}
