// draw_command.h
#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
using namespace DirectX;

#include <vector>
class ShaderProgramResource;

// 2D描画コマンドのインスタンス情報
struct DrawCommand2DInstance {
    XMFLOAT2 position;
    XMFLOAT2 size;
    float    angleZ;
    XMFLOAT4 color;
    XMFLOAT4 uvRect;
};

// 2D描画コマンドのバッチ情報
struct DrawBatch2D {
    int orderInLayer;
    ID3D11ShaderResourceView* texture;
    ShaderProgramResource* shaderProgram;

    std::vector<DrawCommand2DInstance> instances;
    
    DrawBatch2D() : orderInLayer(0), texture(nullptr), shaderProgram(nullptr) {
        instances.reserve(1024);
    }
};

struct DrawCommand3DInstance {
    XMFLOAT3 position;
    XMFLOAT3 rotation;
    XMFLOAT3 scale;
    XMFLOAT4 color;
    XMFLOAT4 uvRect;
};

struct DrawBatch3D {
    ID3D11ShaderResourceView* texture;
    ShaderProgramResource* shaderProgram;

    std::vector<DrawCommand3DInstance> instances;

    DrawBatch3D() : texture(nullptr), shaderProgram(nullptr) {
        instances.reserve(1024);
    }
};