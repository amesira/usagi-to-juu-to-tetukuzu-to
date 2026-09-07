#include "../../Source/Engine/Processor/RenderPass/UiRender/ui_draw_command.h"
#include <cassert>
#include <cmath>
#include <iostream>

using namespace DirectX;
using namespace UiDrawCommand;

static bool Near(float a, float b) { return std::abs(a - b) < 0.0001f; }

int main()
{
    // Only resource identity matters to grouping; these handles are never dereferenced.
    int textures[2] = {};
    int shaders[2] = {};
    auto* texture = reinterpret_cast<ID3D11ShaderResourceView*>(&textures[0]);
    auto* otherTexture = reinterpret_cast<ID3D11ShaderResourceView*>(&textures[1]);
    auto* shader = reinterpret_cast<ShaderProgramResource*>(&shaders[0]);
    auto* otherShader = reinterpret_cast<ShaderProgramResource*>(&shaders[1]);
    std::vector<DrawBatch3D> batches;
    DrawCommand3DInstance command;
    for (int i = 0; i < 4097; ++i) {
        command.position.x = static_cast<float>(i);
        FindOrAddBatch3D(batches, texture, shader).instances.push_back(command);
    }
    assert(batches.size() == 1 && batches[0].instances.size() == 4097);
    FindOrAddBatch3D(batches, otherTexture, shader);
    FindOrAddBatch3D(batches, texture, otherShader);
    assert(batches.size() == 3);
    assert(&FindOrAddBatch3D(batches, texture, shader) == &batches[0]);
    assert(batches[0].instances.back().position.x == 4096.0f);

    // For multiple yaw/pitch/roll camera poses, the entire UI stays in the camera plane.
    for (int i = 0; i < 10; ++i) {
        XMMATRIX camera = XMMatrixRotationRollPitchYaw(i * 0.13f, i * 0.3f, i * 0.07f)
            * XMMatrixTranslation(4.0f, 3.0f, -6.0f);
        XMMATRIX view = XMMatrixInverse(nullptr, camera);
        XMMATRIX billboard = XMMatrixInverse(nullptr, view);
        billboard.r[3] = XMVectorSet(0, 0, 0, 1);
        command.position = { 2, 5, 9 };
        command.offset = { -0.5f, -0.2f };
        command.scale = { 2, 1, 1 };
        XMMATRIX world = MakeBillboardWorld(command, billboard);
        XMFLOAT3 right, down, center, anchor;
        XMStoreFloat3(&right, XMVector3TransformNormal(XMVectorSet(1,0,0,0), world * view));
        XMStoreFloat3(&down, XMVector3TransformNormal(XMVectorSet(0,1,0,0), world * view));
        XMStoreFloat3(&center, XMVector3TransformCoord(XMVectorZero(), world * view));
        XMStoreFloat3(&anchor, XMVector3TransformCoord(XMLoadFloat3(&command.position), view));
        assert(Near(right.x, 2) && Near(right.y, 0) && Near(right.z, 0));
        assert(Near(down.x, 0) && Near(down.y, -1) && Near(down.z, 0));
        assert(Near(center.x - anchor.x, -0.5f));
        assert(Near(center.y - anchor.y, 0.2f));
        assert(Near(center.z, anchor.z));

        // A half-filled slider must retain exactly the same left edge as its background.
        command.offset = {};
        const XMMATRIX background = MakeBillboardWorld(command, billboard);
        command.offset.x = -command.scale.x * 0.25f;
        command.scale.x *= 0.5f;
        const XMMATRIX fill = MakeBillboardWorld(command, billboard);
        const XMVECTOR edge = XMVectorSet(-0.5f, 0, 0, 1);
        const XMVECTOR difference = XMVector3TransformCoord(edge, background)
            - XMVector3TransformCoord(edge, fill);
        assert(XMVectorGetX(XMVector3Length(difference)) < 0.0001f);
    }
    std::cout << "UI batch grouping, 4097 submissions, billboard poses and slider alignment passed.\n";
}