#define STB_TRUETYPE_IMPLEMENTATION
#include "Source/Engine/Graphics/font_repository.h"
#include <cassert>
#include <iostream>

using Microsoft::WRL::ComPtr;

int main()
{
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    const HRESULT hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0,
        nullptr, 0, D3D11_SDK_VERSION, device.GetAddressOf(), nullptr, context.GetAddressOf());
    assert(SUCCEEDED(hr));
    FontRepository repository;
    repository.Initialize(device.Get(), context.Get());
    auto* font = repository.GetFontResource("asset/Font/PixelMplus12-Regular.ttf");
    assert(font && font->atlasSRV);
    assert(font == repository.GetFontResource("asset/Font/../Font/PixelMplus12-Regular.ttf"));
    assert(font == repository.GetFontResource(std::filesystem::absolute("asset/Font/PixelMplus12-Regular.ttf")));
    assert(font == repository.GetFontResource({}));
    assert(font == repository.GetFontResource("asset/Font/nonexistent_font_for_test.ttf"));
    assert(font == repository.GetFontResource("asset/Font/nonexistent_font_for_test.ttf"));

    const auto* digit = repository.GetGlyph(*font, '8');
    assert(digit && digit->width > 0 && digit->x_advance > 0);
    const int cursor = font->atlasCursorX;
    assert(digit == repository.GetGlyph(*font, '8'));
    assert(font->atlasCursorX == cursor);
    const auto* space = repository.GetGlyph(*font, ' ');
    assert(space && space->width == 0 && space->x_advance > 0);
    assert(font->atlasCursorX == cursor);

    auto* other = repository.GetFontResource("asset/Font/nagino.otf");
    assert(other && other != font && other->atlasSRV.Get() != font->atlasSRV.Get());
    assert(other->atlasCursorX == 0 && other->atlasCursorY == 0);
    assert(repository.GetGlyph(*other, '8'));
    assert(font->atlasCursorX == cursor);

    // Verify that glyph pixels were actually uploaded and unused atlas texels are clear.
    D3D11_TEXTURE2D_DESC desc;
    font->atlasTexture->GetDesc(&desc);
    desc.Usage = D3D11_USAGE_STAGING;
    desc.BindFlags = 0;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    ComPtr<ID3D11Texture2D> staging;
    assert(SUCCEEDED(device->CreateTexture2D(&desc, nullptr, staging.GetAddressOf())));
    context->CopyResource(staging.Get(), font->atlasTexture.Get());
    D3D11_MAPPED_SUBRESOURCE mapped = {};
    assert(SUCCEEDED(context->Map(staging.Get(), 0, D3D11_MAP_READ, 0, &mapped)));
    size_t sum = 0;
    auto* pixels = static_cast<const unsigned char*>(mapped.pData);
    for (int y = 0; y < static_cast<int>(digit->height); ++y)
        for (int x = 0; x < static_cast<int>(digit->width); ++x)
            sum += pixels[y * mapped.RowPitch + x];
    assert(sum > 0);
    assert(pixels[(desc.Height - 1) * mapped.RowPitch + desc.Width - 1] == 0);
    context->Unmap(staging.Get(), 0);

    font->atlasCursorX = FontResource::ATLAS_WIDTH;
    font->atlasCursorY = FontResource::ATLAS_HEIGHT;
    assert(!repository.GetGlyph(*font, '9'));
    assert(font->atlasFullReported && font->unavailableGlyphs.count('9'));
    assert(digit == repository.GetGlyph(*font, '8'));
    repository.Finalize();
    assert(!repository.GetFontResource({}));
    std::cout << "Font path cache, TTF/OTF atlases, fallback, glyph reuse, GPU upload and atlas-full handling passed.\n";
}