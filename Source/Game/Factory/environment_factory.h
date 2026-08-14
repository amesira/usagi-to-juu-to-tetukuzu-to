//---------------------------------------------------
// environment_factory.h
// 
// Author：Miu Kitamura
// Date  ：2026/04/14
//---------------------------------------------------
#ifndef ENVIRONMENT_FACTORY_H
#define ENVIRONMENT_FACTORY_H
#include <DirectXMath.h>
using namespace DirectX;

class GameObject;
class SceneBase;
class IScene;

namespace EnvironmentFactory {
    // カメラ生成
    GameObject* CreateCamera(SceneBase* scene, const XMFLOAT3& position, const XMFLOAT3& atPosition);
    // CameraControlのアタッチ
    bool    AttachCameraControl(GameObject* camera);

    // DirectionalLight生成
    GameObject* CreateDirectionalLight(SceneBase* scene, const XMFLOAT4& direction, const XMFLOAT4& diffuse, const XMFLOAT4& ambient);
    // PointLight生成
    GameObject* CreatePointLight(IScene* scene, const XMFLOAT4& diffuse, float range);

};

#endif // !ENVIRONMENT_FACTORY_H