#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <filesystem>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "External/stb_truetype.h"

struct FontGlyphInfo {
    float x_off = 0, y_off = 0;
    float width = 0, height = 0;
    float x_advance = 0;
    float u0 = 0, v0 = 0, u1 = 0, v1 = 0;
};

// Owned by FontRepository until Finalize. stbtt_fontinfo references fontBuffer.
struct FontResource {
    static constexpr float BASE_FONT_SIZE = 64.0f;
    static constexpr int ATLAS_WIDTH = 1024;
    static constexpr int ATLAS_HEIGHT = 1024;

    FontResource() = default;
    FontResource(const FontResource&) = delete;
    FontResource& operator=(const FontResource&) = delete;

    std::filesystem::path path;
    std::vector<unsigned char> fontBuffer;
    stbtt_fontinfo fontInfo = {};
    std::unordered_map<int, FontGlyphInfo> glyphCache;
    std::unordered_set<int> unavailableGlyphs;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> atlasTexture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> atlasSRV;
    int atlasCursorX = 0;
    int atlasCursorY = 0;
    int currentLineHeight = 0;
    bool atlasFullReported = false;
};
