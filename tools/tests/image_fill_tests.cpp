#include "Engine/Processor/RenderPass/UiRender/image_fill_utility.h"
#include <cassert>
#include <limits>
#include <iostream>
using namespace DirectX;
static bool Near(float a, float b) { return std::abs(a-b) < 0.0001f; }
int main()
{
    ImageComponent image;
    const XMFLOAT4 atlas = {0.2f,0.3f,0.4f,0.6f};
    image.SetUvRect(atlas);
    for (const auto method : {ImageComponent::FillMethod::Horizontal, ImageComponent::FillMethod::Vertical}) {
        image.SetFillMethod(method);
        for (bool reverse : {false,true}) {
            image.SetFillReverse(reverse);
            for (float amount : {0.0f,0.25f,0.5f,1.0f}) {
                image.SetFillAmount(amount);
                for (float angle : {0.0f, XM_PIDIV2, 0.7f}) {
                    UiDrawCommand::DrawCommand2DInstance d = {};
                    d.position={30,40}; d.size={200,80}; d.angleZ=angle; d.uvRect=atlas;
                    UiDrawCommand::DrawCommand3DInstance w;
                    w.position={2,3,4}; w.offset={5,6}; w.scale={200,80,2}; w.uvRect=atlas;
                    assert(ImageFillUtility::Apply(image,d) == (amount>0));
                    assert(ImageFillUtility::Apply(image,w) == (amount>0));
                    if (amount==0) continue;
                    const bool horizontal = method==ImageComponent::FillMethod::Horizontal;
                    const float edge = horizontal ? (reverse ? 0.5f : -0.5f) : (reverse ? -0.5f : 0.5f);
                    const float originalX = horizontal ? edge*200 : 0;
                    const float originalY = horizontal ? 0 : edge*80;
                    const float newX = horizontal ? edge*d.size.x : 0;
                    const float newY = horizontal ? 0 : edge*d.size.y;
                    const float c=std::cos(angle),s=std::sin(angle);
                    assert(Near(d.position.x+newX*c-newY*s,30+originalX*c-originalY*s));
                    assert(Near(d.position.y+newX*s+newY*c,40+originalX*s+originalY*c));
                    assert(Near(d.size.x,w.scale.x) && Near(d.size.y,w.scale.y));
                    assert(Near(w.offset.x+newX,5+originalX));
                    assert(Near(w.offset.y+newY,6+originalY));
                    assert(Near(w.position.x,2) && Near(w.scale.z,2));
                    if (horizontal) {
                        assert(Near(d.uvRect.z,0.4f*amount));
                        assert(Near(d.uvRect.x,reverse ? 0.2f+0.4f*(1-amount) : 0.2f));
                        assert(Near(d.uvRect.y,0.3f) && Near(d.uvRect.w,0.6f));
                    } else {
                        assert(Near(d.uvRect.w,0.6f*amount));
                        assert(Near(d.uvRect.y,reverse ? 0.3f : 0.3f+0.6f*(1-amount)));
                        assert(Near(d.uvRect.x,0.2f) && Near(d.uvRect.z,0.4f));
                    }
                    assert(Near(d.uvRect.x,w.uvRect.x) && Near(d.uvRect.y,w.uvRect.y));
                    assert(Near(image.GetUvRect().z,0.4f));
                }
            }
        }
    }
    image.SetFillAmount(-1); assert(image.GetFillAmount()==0);
    image.SetFillAmount(2); assert(image.GetFillAmount()==1);
    image.SetFillAmount(std::numeric_limits<float>::quiet_NaN()); assert(image.GetFillAmount()==0);
    image.SetFillMethod(ImageComponent::FillMethod::None);
    UiDrawCommand::DrawCommand2DInstance d = {};
    d.size={200,80}; d.uvRect=atlas;
    assert(ImageFillUtility::Apply(image,d));
    assert(d.size.x==200 && d.size.y==80 && Near(d.uvRect.z,0.4f));
    // Mirrored UVs retain their signed span when clipped.
    image.SetFillMethod(ImageComponent::FillMethod::Horizontal);
    image.SetFillAmount(0.5f); image.SetFillReverse(true);
    d.uvRect={0.8f,0.3f,-0.4f,0.6f};
    assert(ImageFillUtility::Apply(image,d));
    assert(Near(d.uvRect.x,0.6f) && Near(d.uvRect.z,-0.2f));
    std::cout << "Image Fill: four directions, 0/partial/full, rotated fixed edges, 3D offsets, atlas/mirrored UVs and clamping passed.\n";
}