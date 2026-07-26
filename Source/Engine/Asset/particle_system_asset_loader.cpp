//===================================================
// File  ：Engine/Asset/particle_system_asset_loader.cpp
// Date  ：2026/07/20
// Author：Miu Kitamura
// 
// ・ParticleSystemAssetのセーブ/ロードをJSON形式で行う
//===================================================
#include "particle_system_asset_loader.h"
#include "particle_system_data.h"
#include "particle_system_schema.h"

#include "Engine/Asset/Schema/enum_field_serializer.h"
#include "Engine/Asset/Schema/field_serializer.h"

#include "Utility/mi_curve_json.h"
#include "Utility/mi_math_json.h"

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

namespace
{
    using json = nlohmann::json;

#pragma region MinMaxFloat/MinMaxColorのJSONシリアライズ/デシリアライズ
    // MinMaxFloatをJSONにシリアライズする
    json SerializeMinMaxFloat(const ParticleSystemData::MinMaxFloat& value)
    {
        return {
            { "randomBetweenTwoConstants", value.randomBetweenTwoConstants },
            { "constant", value.constant },
            { "constantMin", value.constantMin },
            { "constantMax", value.constantMax },
        };
    }

    // MinMaxFloatをJSONからデシリアライズする
    bool DeserializeMinMaxFloat(const json& jsonValue, ParticleSystemData::MinMaxFloat& outValue)
    {
        if (!jsonValue.is_object())
        {
            return false;
        }

        outValue.randomBetweenTwoConstants = jsonValue.value(
            "randomBetweenTwoConstants", outValue.randomBetweenTwoConstants);
        outValue.constant = jsonValue.value("constant", outValue.constant);
        outValue.constantMin = jsonValue.value("constantMin", outValue.constantMin);
        outValue.constantMax = jsonValue.value("constantMax", outValue.constantMax);
        return true;
    }

    // MinMaxColorをJSONにシリアライズする
    json SerializeMinMaxColor(const ParticleSystemData::MinMaxColor& value)
    {
        return {
            { "randomBetweenTwoColors", value.randomBetweenTwoColors },
            { "color", MiMathJson::SerializeFloat4(value.color) },
            { "colorMin", MiMathJson::SerializeFloat4(value.colorMin) },
            { "colorMax", MiMathJson::SerializeFloat4(value.colorMax) },
        };
    }

    // MinMaxColorをJSONからデシリアライズする
    bool DeserializeMinMaxColor(const json& jsonValue, ParticleSystemData::MinMaxColor& outValue)
    {
        if (!jsonValue.is_object())
        {
            return false;
        }

        outValue.randomBetweenTwoColors = jsonValue.value(
            "randomBetweenTwoColors", outValue.randomBetweenTwoColors);

        if (const auto it = jsonValue.find("color");
            it != jsonValue.end() && !MiMathJson::DeserializeFloat4(*it, outValue.color))
        {
            return false;
        }
        if (const auto it = jsonValue.find("colorMin");
            it != jsonValue.end() && !MiMathJson::DeserializeFloat4(*it, outValue.colorMin))
        {
            return false;
        }
        if (const auto it = jsonValue.find("colorMax");
            it != jsonValue.end() && !MiMathJson::DeserializeFloat4(*it, outValue.colorMax))
        {
            return false;
        }
        return true;
    }
#pragma endregion

#pragma region EnumのJSONシリアライズ/デシリアライズ
    // SimulationSpaceの文字列化
    const char* ToString(ParticleSystemData::SimulationSpace value)
    {
        switch (value)
        {
        case ParticleSystemData::SimulationSpace::Local: return "Local";
        case ParticleSystemData::SimulationSpace::World: return "World";
        }
        return "Local";
    }

    // ShapeTypeの文字列化
    const char* ToString(ParticleSystemData::ShapeType value)
    {
        switch (value)
        {
        case ParticleSystemData::ShapeType::Sphere: return "Sphere";
        case ParticleSystemData::ShapeType::Cone: return "Cone";
        }
        return "Sphere";
    }

    // BillboardModeの文字列化
    const char* ToString(ParticleSystemData::BillboardMode value)
    {
        switch (value)
        {
        case ParticleSystemData::BillboardMode::View: return "View";
        case ParticleSystemData::BillboardMode::Horizontal: return "Horizontal";
        }
        return "View";
    }

    // BlendModeの文字列化
    const char* ToString(ParticleSystemData::BlendMode value)
    {
        switch (value)
        {
        case ParticleSystemData::BlendMode::AlphaBlend: return "AlphaBlend";
        case ParticleSystemData::BlendMode::Additive: return "Additive";
        }
        return "AlphaBlend";
    }

    // SimulationSpaceをJSONからデシリアライズする
    bool DeserializeSimulationSpace(const json& value, ParticleSystemData::SimulationSpace& outValue)
    {
        if (!value.is_string()) return false;
        const std::string text = value.get<std::string>();
        if (text == "Local") outValue = ParticleSystemData::SimulationSpace::Local;
        else if (text == "World") outValue = ParticleSystemData::SimulationSpace::World;
        else return false;
        return true;
    }

    // ShapeTypeをJSONからデシリアライズする
    bool DeserializeShapeType(const json& value, ParticleSystemData::ShapeType& outValue)
    {
        if (!value.is_string()) return false;
        const std::string text = value.get<std::string>();
        if (text == "Sphere") outValue = ParticleSystemData::ShapeType::Sphere;
        else if (text == "Cone") outValue = ParticleSystemData::ShapeType::Cone;
        else return false;
        return true;
    }

    // BillboardModeをJSONからデシリアライズする
    bool DeserializeBillboardMode(const json& value, ParticleSystemData::BillboardMode& outValue)
    {
        if (!value.is_string()) return false;
        const std::string text = value.get<std::string>();
        if (text == "View") outValue = ParticleSystemData::BillboardMode::View;
        else if (text == "Horizontal") outValue = ParticleSystemData::BillboardMode::Horizontal;
        else return false;
        return true;
    }

    // BlendModeをJSONからデシリアライズする
    bool DeserializeBlendMode(const json& value, ParticleSystemData::BlendMode& outValue)
    {
        if (!value.is_string()) return false;
        const std::string text = value.get<std::string>();
        if (text == "AlphaBlend") outValue = ParticleSystemData::BlendMode::AlphaBlend;
        else if (text == "Additive") outValue = ParticleSystemData::BlendMode::Additive;
        else return false;
        return true;
    }
#pragma endregion

#pragma region ModuleのJSONシリアライズ/デシリアライズ
    // MainModuleをJSONにシリアライズする
    json SerializeMain(const ParticleSystemData::MainModule& module)
    {
        return {
            { "duration", module.duration },
            { "loop", module.loop },
            { "playOnAwake", module.playOnAwake },
            { "startLifetime", SerializeMinMaxFloat(module.startLifetime) },
            { "startSpeed", SerializeMinMaxFloat(module.startSpeed) },
            { "startSize", SerializeMinMaxFloat(module.startSize) },
            { "startColor", SerializeMinMaxColor(module.startColor) },
            { "gravity", MiMathJson::SerializeFloat3(module.gravity) },
            { "simulationSpeed", module.simulationSpeed },
            { "simulationSpace", ToString(module.simulationSpace) },
        };
    }

    // MainModuleをJSONからデシリアライズする
    bool DeserializeMain(const json& jsonValue, ParticleSystemDesc& desc)
    {
        auto& module = desc.mainModule;
        module.duration = jsonValue.value("duration", module.duration);
        module.loop = jsonValue.value("loop", module.loop);
        module.playOnAwake = jsonValue.value("playOnAwake", module.playOnAwake);
        module.simulationSpeed = jsonValue.value("simulationSpeed", module.simulationSpeed);

        if (const auto it = jsonValue.find("simulationSpace");
            it != jsonValue.end() && !DeserializeSimulationSpace(*it, module.simulationSpace)) return false;
        if (const auto it = jsonValue.find("startLifetime");
            it != jsonValue.end() && !DeserializeMinMaxFloat(*it, module.startLifetime)) return false;
        if (const auto it = jsonValue.find("startSpeed");
            it != jsonValue.end() && !DeserializeMinMaxFloat(*it, module.startSpeed)) return false;
        if (const auto it = jsonValue.find("startSize");
            it != jsonValue.end() && !DeserializeMinMaxFloat(*it, module.startSize)) return false;
        if (const auto it = jsonValue.find("startColor");
            it != jsonValue.end() && !DeserializeMinMaxColor(*it, module.startColor)) return false;
        if (const auto it = jsonValue.find("gravity");
            it != jsonValue.end() && !MiMathJson::DeserializeFloat3(*it, module.gravity)) return false;
        return true;
    }

    // EmissionModuleをJSONにシリアライズする
    json SerializeEmission(const ParticleSystemData::EmissionModule& module)
    {
        return FieldSerialization::SerializeFields(
            module,
            ParticleSystemSchema::GetEmissionSchema());
    }

    // EmissionModuleをJSONからデシリアライズする
    bool DeserializeEmission(const json& jsonValue, ParticleSystemDesc& desc)
    {
        return FieldSerialization::DeserializeFields(
            jsonValue,
            desc.emissionModule,
            ParticleSystemSchema::GetEmissionSchema());
    }

    // ShapeModuleをJSONにシリアライズする
    json SerializeShape(const ParticleSystemData::ShapeModule& module)
    {
        return {
            { "enabled", module.enabled },
            { "type", ToString(module.shapeType) },
            { "sphere", {
                { "radius", module.sphere.radius },
                { "emitFromShell", module.sphere.emitFromShell },
            } },
            { "cone", {
                { "angle", module.cone.angle },
                { "radius", module.cone.radius },
                { "length", module.cone.length },
                { "emitFromBase", module.cone.emitFromBase },
            } },
            { "randomDirectionAmount", module.randomDirectionAmount },
        };
    }

    // ShapeModuleをJSONからデシリアライズする
    bool DeserializeShape(const json& jsonValue, ParticleSystemDesc& desc)
    {
        auto& module = desc.shapeModule;
        module.enabled = jsonValue.value("enabled", module.enabled);
        module.randomDirectionAmount = jsonValue.value(
            "randomDirectionAmount", module.randomDirectionAmount);

        if (const auto it = jsonValue.find("type");
            it != jsonValue.end() && !DeserializeShapeType(*it, module.shapeType)) return false;

        if (const auto it = jsonValue.find("sphere"); it != jsonValue.end())
        {
            if (!it->is_object()) return false;
            module.sphere.radius = it->value("radius", module.sphere.radius);
            module.sphere.emitFromShell = it->value("emitFromShell", module.sphere.emitFromShell);
        }
        if (const auto it = jsonValue.find("cone"); it != jsonValue.end())
        {
            if (!it->is_object()) return false;
            module.cone.angle = it->value("angle", module.cone.angle);
            module.cone.radius = it->value("radius", module.cone.radius);
            module.cone.length = it->value("length", module.cone.length);
            module.cone.emitFromBase = it->value("emitFromBase", module.cone.emitFromBase);
        }
        return true;
    }

    // SizeOverLifetimeModuleをJSONにシリアライズする
    json SerializeSizeOverLifetime(const ParticleSystemData::SizeOverLifetimeModule& module)
    {
        return {
            { "enabled", module.enabled },
            { "size", MiCurveJson::Serialize(module.size) },
        };
    }

    // SizeOverLifetimeModuleをJSONからデシリアライズする
    bool DeserializeSizeOverLifetime(const json& jsonValue, ParticleSystemDesc& desc)
    {
        auto& module = desc.sizeOverLifetimeModule;
        module.enabled = jsonValue.value("enabled", module.enabled);
        if (const auto it = jsonValue.find("size");
            it != jsonValue.end() && !MiCurveJson::Deserialize(*it, module.size)) return false;
        return true;
    }

    // TextureSheetAnimationModuleをJSONにシリアライズする
    json SerializeTextureSheetAnimation(const ParticleSystemData::TextureSheetAnimation& module)
    {
        return FieldSerialization::SerializeFields(
            module,
            ParticleSystemSchema::GetTextureSheetAnimationSchema());
    }

    // TextureSheetAnimationModuleをJSONからデシリアライズする
    bool DeserializeTextureSheetAnimation(const json& jsonValue, ParticleSystemDesc& desc)
    {
        return FieldSerialization::DeserializeFields(
            jsonValue,
            desc.textureSheetAnimation,
            ParticleSystemSchema::GetTextureSheetAnimationSchema());
    }

    // RendererModuleをJSONにシリアライズする
    json SerializeRenderer(const ParticleSystemData::RendererModule& module)
    {
        return {
            { "texturePath", module.texturePath },
            { "uvRect", MiMathJson::SerializeFloat4(module.uvRect) },
            { "billboardMode", ToString(module.billboardMode) },
            { "blendMode", ToString(module.blendMode) },
            { "sortByDistance", module.sortByDistance },
        };
    }

    // RendererModuleをJSONからデシリアライズする
    bool DeserializeRenderer(const json& jsonValue, ParticleSystemDesc& desc)
    {
        auto& module = desc.rendererModule;
        module.texturePath = jsonValue.value("texturePath", module.texturePath);
        module.sortByDistance = jsonValue.value("sortByDistance", module.sortByDistance);
        if (const auto it = jsonValue.find("uvRect");
            it != jsonValue.end() && !MiMathJson::DeserializeFloat4(*it, module.uvRect)) return false;
        if (const auto it = jsonValue.find("billboardMode");
            it != jsonValue.end() && !DeserializeBillboardMode(*it, module.billboardMode)) return false;
        if (const auto it = jsonValue.find("blendMode");
            it != jsonValue.end() && !DeserializeBlendMode(*it, module.blendMode)) return false;

        // textureResourceは保存データではないため、参照解決前は空にする。
        module.textureResource = nullptr;
        return true;
    }
#pragma endregion

    /// @brief  JSONから指定のモジュールをデシリアライズする
    /// @param data     元のJSONデータ
    /// @param key      モジュールを示すキー
    /// @param desc     デシリアライズ先のParticleSystemDesc
    /// @param deserialize モジュールのデシリアライズ関数
    /// @return 
    bool DeserializeModule(
        const json& data,
        const char* key,
        ParticleSystemDesc& desc,
        bool (*deserialize)(const json&, ParticleSystemDesc&))
    {
        const auto it = data.find(key);
        if (it == data.end()) return true;
        return it->is_object() && deserialize(*it, desc);
    }
}

/// @brief パーティクルアセットをセーブする
bool ParticleSystemAssetLoader::SaveParticle(
    const std::string& filePath,
    const ParticleSystemAsset& asset)
{
    const ParticleSystemDesc& desc = asset.GetDesc();
    const AssetHeader& header = asset.GetHeader();

    // JSON形式にシリアライズ化
    json root = {
        { "type", "ParticleSystem" },
        { "formatVersion", m_supportedVersion },
        { "name", header.m_name },
        { "data", {
            { "main", SerializeMain(desc.mainModule) },
            { "emission", SerializeEmission(desc.emissionModule) },
            { "shape", SerializeShape(desc.shapeModule) },
            { "sizeOverLifetime", SerializeSizeOverLifetime(desc.sizeOverLifetimeModule) },
            { "textureSheetAnimation", SerializeTextureSheetAnimation(desc.textureSheetAnimation) },
            { "renderer", SerializeRenderer(desc.rendererModule) },
        } },
    };

    // === 指定のパスにファイルを保存 ===
    std::ofstream ofs(filePath);
    if (!ofs.is_open()) return false;
    try
    {
        // ParticleAssetファイルを出力する
        ofs << root.dump(4) << '\n';
        return ofs.good();
    }
    catch (const json::exception& error)
    {
        // JSONの解析に失敗
        return false;
    }
}

/// @brief パーティクルアセットをロードする
bool ParticleSystemAssetLoader::LoadParticle(
    const std::string& filePath,
    ParticleSystemAsset& outAsset)
{
    // === アセットファイルを開く ===
    std::ifstream ifs(filePath);
    if (!ifs.is_open()) return false;

    try
    {
        json root;
        ifs >> root;

        // Typeを確認
        if (!root.is_object() || root.value("type", "") != "ParticleSystem") return false;

        // FormatVersionを確認
        const int version = root.value("formatVersion", 0);
        if (version != m_supportedVersion) return false;

        // Dataを確認
        const auto dataIt = root.find("data");
        if (dataIt == root.end() || !dataIt->is_object()) return false;

        // JSONからParticlesystemAssetをデシリアライズ
        ParticleSystemDesc loadedDesc{};
        if (!DeserializeModule(*dataIt, "main", loadedDesc, DeserializeMain) ||
            !DeserializeModule(*dataIt, "emission", loadedDesc, DeserializeEmission) ||
            !DeserializeModule(*dataIt, "shape", loadedDesc, DeserializeShape) ||
            !DeserializeModule(*dataIt, "sizeOverLifetime", loadedDesc, DeserializeSizeOverLifetime) ||
            !DeserializeModule(*dataIt, "textureSheetAnimation", loadedDesc, DeserializeTextureSheetAnimation) ||
            !DeserializeModule(*dataIt, "renderer", loadedDesc, DeserializeRenderer))
        {
            return false;
        }

        // ヘッダーを設定
        AssetHeader loadedHeader;
        loadedHeader.m_type = "ParticleSystem";
        loadedHeader.m_formatVersion = version;
        loadedHeader.m_name = root.value("name", "UnnamedParticleSystem");

        // === 全項目の読み込み成功後に反映 ===
        outAsset.SetHeader(loadedHeader);
        outAsset.SetFilePath(filePath);
        outAsset.SetDesc(loadedDesc);
        return true;
    }
    catch (const json::exception& error)
    {
        // JSONの解析に失敗
        return false;
    }
}

/// @brief ParticleSystemAssetを再読み込みする。キャッシュにある場合は上書きする
bool ParticleSystemAssetLoader::ReloadParticle(
    const std::string& filePath)
{
    auto it = m_particleAssetCache.find(filePath);
    if (it == m_particleAssetCache.end()) {
        // キャッシュに無い場合は読み込まない
        return false;
    }

    ParticleSystemAsset newAsset;
    if (LoadParticle(filePath, newAsset)) {
        // 成功した場合はキャッシュを更新
        it->second = std::make_unique<ParticleSystemAsset>(std::move(newAsset));
        return true;
    }

    return false;
}

/// @brief ParticleSystemAssetを取得する。キャッシュに無い場合はロードする
ParticleSystemAsset* ParticleSystemAssetLoader::GetParticle(const std::string& filePath)
{
    auto it = m_particleAssetCache.find(filePath);
    if (it != m_particleAssetCache.end()) {
        return it->second.get();
    }

    // キャッシュに無い場合はロードする
    auto newAsset = std::make_unique<ParticleSystemAsset>();
    if (LoadParticle(filePath, *newAsset)) {
        ParticleSystemAsset* assetPtr = newAsset.get();
        m_particleAssetCache[filePath] = std::move(newAsset);
        return assetPtr;
    }

    return nullptr;
}
