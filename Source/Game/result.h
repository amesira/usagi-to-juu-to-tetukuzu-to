#ifndef RESULT_H
#define RESULT_H

#include "Engine/Core/scene_base.h"

// リザルトシーンの生成・破棄を担当する。
class ResultScene : public SceneBase {
public:
    void Initialize() override;
    void Finalize() override;
    void Update() override;
    void Draw() override;
};

#endif // RESULT_H
