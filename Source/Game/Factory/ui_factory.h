//---------------------------------------------------
// ui_factory.h
// 
// Author：Miu Kitamura
// Date  ：2026/04/14
//---------------------------------------------------
#ifndef UI_FACTORY_H
#define UI_FACTORY_H
#include <DirectXMath.h>
using namespace DirectX;
#include <string>
#include "Game/PresBehavior/UI/ui_handle.h"

class GameObject;
class IScene;

namespace UiFactory {
    // Behavior内部で生成・保持するためのハンドル版。
    UiHandle CreateUiImageHandle(IScene* scene, const std::wstring& texturePath);
    UiHandle CreateUiTextHandle(IScene* scene, const std::u8string& text);
    UiHandle CreateUiSliderHandle(IScene* scene, const XMFLOAT4& bgColor, const XMFLOAT4& fillColor, float value);
    // UIイメージ生成
    GameObject* CreateUiImage(IScene* scene, const std::wstring& texturePath);
    // UIテキスト生成
    GameObject* CreateUiText(IScene* scene, const std::u8string& text);
    // UIスライダー生成
    GameObject* CreateUiSlider(IScene* scene, const XMFLOAT4& bgColor, const XMFLOAT4& fillColor, float value);
    // UITransformセットアップ
    bool    SetupUiTransform(GameObject* uiElement, const XMFLOAT2& position, const XMFLOAT2& size);

};

#endif // !UI_FACTORY_H
