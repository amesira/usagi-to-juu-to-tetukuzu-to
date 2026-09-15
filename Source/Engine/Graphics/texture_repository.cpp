//===================================================
// texture_repository.cpp
// 
// Author：Miu Kitamura
// Date  ：2026/03/29
//===================================================
#include "texture_repository.h"

#include <memory>
#include "Engine/Device/direct3d.h"

#include "Utility/debug_ostream.h"

// テクスチャリポジトリの初期化
void TextureRepository::Initialize() 
{
    m_pDevice = Direct3D_GetDevice();
    m_pContext = Direct3D_GetDeviceContext();

    m_textureCache.clear();
}

// テクスチャリポジトリの終了処理
void TextureRepository::Finalize() 
{
    m_textureCache.clear();
}

// テクスチャの生成
TextureResource* TextureRepository::GenerateTextureResource(const TextureResource& texture)
{
    const auto cacheKey = texture.sourcePath.lexically_normal();
    // キャッシュを確認し、既に存在する場合は上書きする
    auto it = m_textureCache.find(cacheKey);
    if (it != m_textureCache.end())
    {
        it->second = std::make_unique<TextureResource>(texture);
        it->second->sourcePath = cacheKey;
        return it->second.get();
    }

    // キャッシュに存在しない場合は新規に追加する
    m_textureCache[cacheKey] = std::make_unique<TextureResource>(texture);
    m_textureCache[cacheKey]->sourcePath = cacheKey;
    return m_textureCache[cacheKey].get();
}

// テクスチャの取得。キャッシュに無い場合は読み込む。
TextureResource* TextureRepository::GetTextureResource(const std::filesystem::path& filePath)
{
    const auto cacheKey = filePath.lexically_normal();
    // キャッシュを確認
    auto it = m_textureCache.find(cacheKey);
    if (it != m_textureCache.end())
    {
        return it->second.get();
    }

    // キャッシュに無い場合は読み込む
    return LoadTexture(cacheKey);
}

//-------------------------------------

// テクスチャの読み込み
TextureResource* TextureRepository::LoadTexture(const std::filesystem::path& filePath)
{
    const auto cacheKey = filePath.lexically_normal();
    auto textureResource = std::make_unique<TextureResource>();
    textureResource->sourcePath = cacheKey;
    
    // テクスチャの読み込み
    TexMetadata metadata;
    ScratchImage image;
    LoadFromWICFile(cacheKey.c_str(), WIC_FLAGS_NONE, &metadata, image);
    CreateShaderResourceView(m_pDevice, image.GetImages(), image.GetImageCount(), metadata, textureResource->texture.GetAddressOf());

    // 読み込み失敗時のエラーチェック
    if (textureResource->texture == nullptr)
    {
        return nullptr;
    }

    TextureResource* result = textureResource.get();
    m_textureCache[cacheKey] = std::move(textureResource);
    return result;
}

// テクスチャの解放
void TextureRepository::ReleaseTexture(const std::filesystem::path& filePath)
{
    auto it = m_textureCache.find(filePath.lexically_normal());
    if (it != m_textureCache.end())
    {
        // ComPtrが自動的にリソースを解放するので、特になし
        m_textureCache.erase(it);
    }
}
