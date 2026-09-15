//---------------------------------------------------
// actor_factory.h
// 
// Author：Miu Kitamura
// Date  ：2026/04/14
//---------------------------------------------------
#ifndef ACTOR_FACTORY_H
#define ACTOR_FACTORY_H
#include <DirectXMath.h>
using namespace DirectX;

class GameObject;
class IScene;
namespace PlayerPrefabSettings { struct Data; }

namespace ActorFactory {
    /// @brief プレイヤー生成
    /// @param position 位置
    GameObject* CreatePlayer(
        IScene* scene,
        const XMFLOAT3& position,
        const PlayerPrefabSettings::Data& settings);

};

#endif // !ACTOR_FACTORY_H
