#include "font_repository.h"
#include <algorithm>
#include <fstream>


void FontRepository::Initialize(ID3D11Device* device, ID3D11DeviceContext* context,
    const std::filesystem::path& root)
{
    Finalize();
    m_device = device;
    m_context = context;
    m_root = std::filesystem::absolute(root).lexically_normal();
}

void FontRepository::Finalize()
{
    m_fontCache.clear();
    m_device = nullptr;
    m_context = nullptr;
}

std::filesystem::path FontRepository::NormalizePath(const std::filesystem::path& path) const
{
    const auto requested = path.empty() ? std::filesystem::path(DEFAULT_FONT_PATH) : path;
    return (requested.is_absolute() ? requested : m_root / requested).lexically_normal();
}

FontResource* FontRepository::GetFontResource(const std::filesystem::path& path)
{
    if (!m_device || !m_context) return nullptr;
    const auto key = NormalizePath(path);
    if (auto* font = FindOrLoad(key)) return font;
    const auto fallback = NormalizePath(DEFAULT_FONT_PATH);
    return key == fallback ? nullptr : FindOrLoad(fallback);
}

FontResource* FontRepository::FindOrLoad(const std::filesystem::path& key)
{
    auto found = m_fontCache.find(key);
    if (found != m_fontCache.end()) return found->second.get();
    auto resource = LoadFont(key);
    if (!resource) OutputDebugStringW((L"Failed to load font: " + key.wstring() + L"\n").c_str());
    auto* result = resource.get();
    m_fontCache.emplace(key, std::move(resource));
    return result;
}

std::unique_ptr<FontResource> FontRepository::LoadFont(const std::filesystem::path& key)
{
    std::ifstream file(key, std::ios::binary | std::ios::ate);
    if (!file) return nullptr;
    const std::streamsize size = file.tellg();
    if (size < 12) return nullptr;
    auto font = std::make_unique<FontResource>();
    font->path = key;
    font->fontBuffer.resize(static_cast<size_t>(size));
    file.seekg(0);
    if (!file.read(reinterpret_cast<char*>(font->fontBuffer.data()), size)) return nullptr;
    // The current API selects the first face for collection fonts.
    const int offset = stbtt_GetFontOffsetForIndex(font->fontBuffer.data(), 0);
    if (offset < 0 || offset >= size || !stbtt_InitFont(&font->fontInfo, font->fontBuffer.data(), offset)) return nullptr;

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = FontResource::ATLAS_WIDTH;
    desc.Height = FontResource::ATLAS_HEIGHT;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    // Clear unused texels and glyph padding to transparent.
    std::vector<unsigned char> pixels(desc.Width * desc.Height, 0);
    D3D11_SUBRESOURCE_DATA initial = {};
    initial.pSysMem = pixels.data();
    initial.SysMemPitch = desc.Width;
    if (FAILED(m_device->CreateTexture2D(&desc, &initial, font->atlasTexture.GetAddressOf()))) return nullptr;
    if (FAILED(m_device->CreateShaderResourceView(font->atlasTexture.Get(), nullptr, font->atlasSRV.GetAddressOf()))) return nullptr;
    return font;
}

const FontGlyphInfo* FontRepository::GetGlyph(FontResource& font, int codepoint)
{
    if (!m_context) return nullptr;
    auto found = font.glyphCache.find(codepoint);
    if (found != font.glyphCache.end()) return &found->second;
    if (font.unavailableGlyphs.count(codepoint)) return nullptr;
    const int index = stbtt_FindGlyphIndex(&font.fontInfo, codepoint);
    if (index == 0) {
        font.unavailableGlyphs.insert(codepoint);
        return nullptr;
    }
    const float scale = stbtt_ScaleForPixelHeight(&font.fontInfo, FontResource::BASE_FONT_SIZE);
    int x0, y0, x1, y1, advance;
    stbtt_GetGlyphBitmapBox(&font.fontInfo, index, scale, scale, &x0, &y0, &x1, &y1);
    stbtt_GetGlyphHMetrics(&font.fontInfo, index, &advance, nullptr);
    FontGlyphInfo glyph;
    glyph.x_off = static_cast<float>(x0);
    glyph.y_off = static_cast<float>(y0);
    glyph.x_advance = advance * scale;
    const int width = x1 - x0;
    const int height = y1 - y0;

    if (width > 0 && height > 0) {
        if (font.atlasCursorX + width > FontResource::ATLAS_WIDTH) {
            font.atlasCursorX = 0;
            font.atlasCursorY += font.currentLineHeight;
            font.currentLineHeight = 0;
        }
        if (width > FontResource::ATLAS_WIDTH || font.atlasCursorY + height > FontResource::ATLAS_HEIGHT) {
            if (!font.atlasFullReported) {
                OutputDebugStringW((L"Font atlas is full: " + font.path.wstring() + L"\n").c_str());
                font.atlasFullReported = true;
            }
            font.unavailableGlyphs.insert(codepoint);
            return nullptr;
        }
        std::vector<unsigned char> pixels(width * height);
        stbtt_MakeGlyphBitmap(&font.fontInfo, pixels.data(), width, height, width, scale, scale, index);
        D3D11_BOX box = {};
        box.left = font.atlasCursorX;
        box.top = font.atlasCursorY;
        box.right = box.left + width;
        box.bottom = box.top + height;
        box.back = 1;
        m_context->UpdateSubresource(font.atlasTexture.Get(), 0, &box, pixels.data(), width, 0);
        glyph.width = static_cast<float>(width);
        glyph.height = static_cast<float>(height);
        glyph.u0 = static_cast<float>(box.left) / FontResource::ATLAS_WIDTH;
        glyph.v0 = static_cast<float>(box.top) / FontResource::ATLAS_HEIGHT;
        glyph.u1 = static_cast<float>(box.right) / FontResource::ATLAS_WIDTH;
        glyph.v1 = static_cast<float>(box.bottom) / FontResource::ATLAS_HEIGHT;
        font.atlasCursorX += width + 1;
        font.currentLineHeight = (std::max)(font.currentLineHeight, height + 1);
    }
    return &font.glyphCache.emplace(codepoint, glyph).first->second;
}
