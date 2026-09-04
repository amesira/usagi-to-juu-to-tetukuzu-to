//---------------------------------------------------
// File  ：_/Asset/LevelAsset/level_data.h
// Date  ：2026/09/02
// Author：Miu Kitamura
//---------------------------------------------------
#pragma once

#include <DirectXMath.h>

#include <string>
#include <vector>

enum class LevelColliderType
{
    None,
    Box,
    Sphere,
};

struct LevelTransformData
{
    DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
    // JSONでは編集しやすい度数法で保持する。
    DirectX::XMFLOAT3 rotationDegrees = { 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 scale = { 1.0f, 1.0f, 1.0f };
};

struct LevelColliderData
{
    LevelColliderType type = LevelColliderType::None;
    DirectX::XMFLOAT3 center = { 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 boxSize = { 1.0f, 1.0f, 1.0f };
    float sphereRadius = 0.5f;
};

struct LevelObjectData
{
    // エディター上でオブジェクトを追跡するための、レベル内で一意なID。
    std::string id;
    std::string name = "GameObject";
    LevelTransformData transform;
    LevelColliderData collider;
    std::string modelPath;
    std::vector<std::string> materialNames;
};

struct LevelData
{
    std::vector<LevelObjectData> objects;
};
