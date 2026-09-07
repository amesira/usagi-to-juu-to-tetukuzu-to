// shader_definitions.h
#pragma once
#include <string>

#include "Engine/Device/direct3d.h"
using namespace DirectX;

namespace ShaderDefinitions {
    enum class VertexType {
        Model,
        SkinnedModel,
        Ui,
        Sprite,
        Particle,

        None,

        DebugLine,
        MAX,
    };

#pragma region Vertex structures
    struct ModelVertex {
        XMFLOAT3 position;
        XMFLOAT3 normal;
        XMFLOAT3 tangent;
        XMFLOAT3 binormal;

        XMFLOAT4 color;
        XMFLOAT2 texCoord;
    };

    struct SkinnedModelVertex {
        XMFLOAT3 position;
        XMFLOAT3 normal;
        XMFLOAT3 tangent;
        XMFLOAT3 binormal;

        XMFLOAT4 color;
        XMFLOAT2 texCoord;

        XMUINT4  boneIndices;
        XMFLOAT4 boneWeights;
    };

    struct DebugLineVertex {
        XMFLOAT3 position;
        XMFLOAT4 color;
    };

    struct UiVertex {
        XMFLOAT3 position;
        XMFLOAT2 texCoord;
    };

    struct SpriteVertex {
        XMFLOAT3 position;
        XMFLOAT3 normal;
        XMFLOAT4 color;
        XMFLOAT2 texCoord;
    };

    struct ParticleVertex {
        XMFLOAT3 position;
        XMFLOAT2 texCoord;
    };

    struct ParticleInstanceData {
        XMMATRIX world;
        XMFLOAT4 color;
        XMFLOAT4 uvRect;
    };

    struct UiInstanceData {
        XMMATRIX world;
        XMFLOAT4 color;
        XMFLOAT4 uvRect;
    };
#pragma endregion

    enum class ShaderBase {
        None,
        Lit,
        SkinnedLit,
        SkinnedUnlit,
        Unlit,
        Ui,
        SpriteLit,
        SpriteUnlit,
        Particle,
        FullScreen,
        DebugLine,
        MAX
    };

    static const std::string SHADER_BASE_NAMES[static_cast<size_t>(ShaderBase::MAX)] = {
        "None",
        "Lit",
        "SkinnedLit",
        "SkinnedUnlit",
        "Unlit",
        "Ui",
        "SpriteLit",
        "SpriteUnlit",
        "Particle",
        "FullScreen",
        "DebugLine",
    };
}
