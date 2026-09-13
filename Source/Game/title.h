#ifndef TITLE_H
#define TITLE_H

#include "Engine/Core/scene_base.h"

// タイトルシーンの生成・破棄を担当する。
class TitleScene : public SceneBase {
public:
    void Initialize() override;
    void Finalize() override;
    void Update() override;
    void Draw() override;
};

#endif // TITLE_H
