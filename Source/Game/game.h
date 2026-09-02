//----------------------------------------------------
// game_scene.h [ゲームシーン制御]
// 
// Author：Miu Kitamura
// Date  ：2025/09/05
//----------------------------------------------------
#ifndef GAME_H
#define GAME_H
#include "Engine/Core/scene_base.h"

class TextComponent;
class ButtonBehavior;
class TpsCameraBehavior;
class ImageComponent;

// ゲームシーン
class GameScene : public SceneBase {
private:

public:
    void    Initialize() override;
    void    Finalize() override;

    void    Update() override;
    void    Draw() override;

};

#endif