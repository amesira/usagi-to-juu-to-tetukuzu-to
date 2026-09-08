// ui_draw_command.h
#pragma once
#include <d3d11.h>
#include <DirectXMath.h>

#include <vector>
class ShaderProgramResource;

namespace UiDrawCommand {
using namespace DirectX;

    // 2D描画コマンドのインスタンス情報
    struct DrawCommand2DInstance {
        XMFLOAT2 position;
        XMFLOAT2 size;
        float    angleZ;
        XMFLOAT4 color;
        XMFLOAT4 uvRect;
        XMFLOAT4 roundFill = {}; // enabled, amount, start radians, direction (+1 clockwise)
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
        XMFLOAT3 position = {}; // World-space anchor; orientation is supplied by the camera.
        XMFLOAT2 offset = {};   // Billboard-local offset (X right, Y down).
        XMFLOAT3 scale = { 1.0f, 1.0f, 1.0f };
        XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
        XMFLOAT4 uvRect = { 0.0f, 0.0f, 1.0f, 1.0f };
        XMFLOAT4 roundFill = {};
    };

    struct DrawBatch3D {
        ID3D11ShaderResourceView* texture;
        ShaderProgramResource* shaderProgram;

        std::vector<DrawCommand3DInstance> instances;

        DrawBatch3D() : texture(nullptr), shaderProgram(nullptr) {
            instances.reserve(1024);
        }
    };

    // 3D描画コマンドのバッチを検索または追加する
    inline DrawBatch3D& FindOrAddBatch3D(std::vector<DrawBatch3D>& batches,
        ID3D11ShaderResourceView* texture, ShaderProgramResource* shader)
    {
        for (auto& batch : batches) {
            // textureとshaderが一致するバッチがあればそれを返す
            if (batch.texture == texture && batch.shaderProgram == shader) return batch;
        }
        auto& batch = batches.emplace_back();
        batch.texture = texture;
        batch.shaderProgram = shader;
        return batch;
    }

    // ビルボード行列を作成する
    inline XMMATRIX MakeBillboardWorld(const DrawCommand3DInstance& instance, const XMMATRIX& billboard)
    {
        return XMMatrixScaling(instance.scale.x, -instance.scale.y, 1.0f)
            * XMMatrixTranslation(instance.offset.x, -instance.offset.y, 0.0f)
            * billboard
            * XMMatrixTranslation(instance.position.x, instance.position.y, instance.position.z);
    }

} // namespace UiDrawCommand
