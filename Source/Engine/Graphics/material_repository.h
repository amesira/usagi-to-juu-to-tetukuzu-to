//---------------------------------------------------
// material_repository.h
// 
// Author：Miu Kitamura
// Date  ：2026/03/29
//---------------------------------------------------
#ifndef MATERIAL_REPOSITORY_H
#define MATERIAL_REPOSITORY_H
#include "material_resource.h"

#include <unordered_map>
#include <memory>

class ConstantBufferResource;

class MaterialRepository {
private:
    ID3D11Device* m_pDevice = nullptr;
    ID3D11DeviceContext* m_pContext = nullptr;

    // デフォルトテクスチャリソース
    TextureResource* m_defaultAlbedoTexture = nullptr;
    TextureResource* m_defaultNormalTexture = nullptr;
    TextureResource* m_defaultEmissiveTexture = nullptr;
    TextureResource* m_defaultAOTexture = nullptr;

    // マテリアルリソースのキャッシュ
    std::unordered_map<std::string, std::unique_ptr<MaterialResource>> m_materialCache;

    // マテリアルの定数バッファ
    ConstantBufferResource* m_materialCB = nullptr;
    // カスタムプロパティ用の定数バッファ
    ConstantBufferResource* m_customPropertyCB = nullptr;

public:
    // 初期化
    void Initialize();
    // 終了
    void Finalize();

    // マテリアルの生成。
    MaterialResource* GenerateMaterial(const MaterialResource& material);
    // マテリアルの取得。キャッシュに無い場合は読み込む。
    MaterialResource* GetMaterial(const std::string& materialName);

    // マテリアルのバインド
    void BindMaterialCB(const MaterialBufferData& material);
    void BindMaterialTexture(const MaterialResource& material);
    void BindMaterialTexture(const MaterialInstance& materialInstance);
    // カスタムプロパティのバインド
    void BindCustomProperties(XMFLOAT4* customPropaties);
    void BindCustomTextures(TextureResource** customTextures);

private:
    // マテリアルの読み込み
    // 現状はマテリアル名をキーとして生成する。
    MaterialResource* LoadMaterial(const std::string& materialName);

    // マテリアルの解放
    void ReleaseMaterial(const std::string& materialName);
};

#endif // MATERIAL_REPOSITORY_H
