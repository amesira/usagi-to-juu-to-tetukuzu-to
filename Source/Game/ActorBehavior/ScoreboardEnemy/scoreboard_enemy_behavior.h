#pragma once
#include <array>
#include <limits>
#include <string>
#include "Engine/Component/behavior_component.h"
#include "Game/ActorBehavior/Enemy/E10_Locomotion/Move/enemy_path_follower.h"
#include "Game/ActorBehavior/Enemy/E00_Core/enemy_context.h"
#include "Game/ActorBehavior/Enemy/enemy_animation_controller.h"
#include "Game/ControllerBehavior/Result/score_save_store.h"

class ScoreboardEnemySettingsAsset;
class TransformComponent;

class ScoreboardEnemyBehavior : public BehaviorComponent {
    static constexpr unsigned int InvalidObjectId = (std::numeric_limits<unsigned int>::max)();
    const ScoreboardEnemySettingsAsset* m_settings = nullptr;
    ScoreRecord m_record;
    std::string m_label;
    TransformComponent* m_transform = nullptr;
    EnemyPathFollower m_pathFollower;
    EnemyContext m_animationContext;
    EnemyAnimationController m_animation;
    std::array<unsigned int, 3> m_uiIds{InvalidObjectId, InvalidObjectId, InvalidObjectId};
    float m_repathTimer = 0;
    bool m_registered = false;
public:
    void Setup(const ScoreboardEnemySettingsAsset* settings, const ScoreRecord& record, std::string label);
    void Start() override;
    void Update() override;
    void OnDestroy() override;
    void DrawComponentInspector() override;
private:
    void CreateBoard();
    void UpdateBoard();
    DirectX::XMFLOAT4 RankColor() const;
    static char RankLetter(int rank);
};
