#pragma once
#include "font_resource.h"
#include <memory>

// Render-thread-only cache shared by 2D/3D text and all render views.
class FontRepository {
public:
    static constexpr const char* DEFAULT_FONT_PATH = "asset/Font/PixelMplus12-Regular.ttf";
    
private:
    ID3D11Device* m_device = nullptr;
    ID3D11DeviceContext* m_context = nullptr;
    std::filesystem::path m_root;

    std::unordered_map<std::filesystem::path, std::unique_ptr<FontResource>> m_fontCache;

public:
    void Initialize(ID3D11Device* device, ID3D11DeviceContext* context,
        const std::filesystem::path& root = std::filesystem::current_path());
    void Finalize();
    FontResource* GetFontResource(const std::filesystem::path& path);
    const FontGlyphInfo* GetGlyph(FontResource& font, int codepoint);

private:
    std::filesystem::path NormalizePath(const std::filesystem::path& path) const;
    FontResource* FindOrLoad(const std::filesystem::path& key);
    std::unique_ptr<FontResource> LoadFont(const std::filesystem::path& key);

};
