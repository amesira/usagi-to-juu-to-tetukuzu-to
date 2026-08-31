#include "player_animation_controller.h"

#include <algorithm>
#include <cctype>
#include <filesystem>

#include "Engine/Component/animation_component.h"
#include "Engine/Component/model_component.h"
#include "Engine/Core/game_object.h"
#include "Game/ActorBehavior/Player/P00_Core/player_context.h"
#include "Game/ActorBehavior/Player/player_behavior.h"

#include "Engine/engine_service_locator.h"
#include "Engine/Graphics/model_repository.h"

namespace {
    /// @brief 指定されたModelComponentのアニメーションクリップの中から、指定されたファイル名に一致するクリップのインデックスを検索する
    int FindClipIndex(const ModelComponent* modelComponent, const std::filesystem::path& fileName)
    {
        if (!modelComponent || !modelComponent->GetModelResource()) return -1;

        const auto& clips = modelComponent->GetModelResource()->animationClips;
        const auto it = std::find_if(clips.begin(), clips.end(), [&fileName](const AnimationClip& clip) {
            return clip.filePath.filename() == fileName;
        });
        return it == clips.end() ? -1 : static_cast<int>(std::distance(clips.begin(), it));
    }

    /// @brief 上半身のボーンのみを有効にしたアニメーションマスクを作成する
    AnimationBoneMask CreateUpperBodyMask(const ModelResource& modelResource)
    {
        const std::array<std::string, 3> upperBodyBones = {
            "Head",
            "Hand.L",
            "Hand.R",
        };

        return AnimationComponent::CreateBoneMask(modelResource, std::vector<std::string>(upperBodyBones.begin(), upperBodyBones.end()));
    }
}

void PlayerAnimationController::Initialize(const PlayerContext& context)
{
    GameObject* player = context.owner ? context.owner->GetOwner() : nullptr;
    if (!player) return;

    m_animationComponent = player->GetComponent<AnimationComponent>();
    ModelComponent* modelComponent = player->GetComponent<ModelComponent>();

    m_frameRequests.clear();
    m_definitionTypes.fill(DefinitionType::None);
    m_clipDefinitions = {};
    m_blendTree1DDefinitions = {};
    m_blendTree2DDefinitions = {};
    m_layerDefinitions = {};
    m_frameLayerRequests.clear();
    m_hasCurrentRequest = false;
    m_waitingForCompletion = false;
    m_requestOrder = 0;
    m_currentSubMachine = SubMachine::Default;

    ModelResource* modelResource = modelComponent ? modelComponent->GetModelResource() : nullptr;
    if (modelResource) {
        Engine::ModelRepository()->LoadAnimation(modelResource, "asset/Model/player_idle.anim.fbx");
        Engine::ModelRepository()->LoadAnimation(modelResource, "asset/Model/player_running.anim.fbx");
        Engine::ModelRepository()->LoadAnimation(modelResource, "asset/Model/player_jump_1.anim.fbx");
        Engine::ModelRepository()->LoadAnimation(modelResource, "asset/Model/player_jump_2.anim.fbx");

        Engine::ModelRepository()->LoadAnimation(modelResource, "asset/Model/player_shotgun_idle.anim.fbx");
        Engine::ModelRepository()->LoadAnimation(modelResource, "asset/Model/player_shotgun_idle_lower.anim.fbx");
        Engine::ModelRepository()->LoadAnimation(modelResource, "asset/Model/player_shotgun_idle_upper.anim.fbx");
        Engine::ModelRepository()->LoadAnimation(modelResource, "asset/Model/player_shotgun_walk_left.anim.fbx");
        Engine::ModelRepository()->LoadAnimation(modelResource, "asset/Model/player_shotgun_walk_forward.anim.fbx");
        Engine::ModelRepository()->LoadAnimation(modelResource, "asset/Model/player_shotgun_walk_right.anim.fbx");
        Engine::ModelRepository()->LoadAnimation(modelResource, "asset/Model/player_shotgun_walk_back.anim.fbx");

        Engine::ModelRepository()->LoadAnimation(modelResource, "asset/Model/player_dual_pistols_rapid_fire.anim.fbx");
        Engine::ModelRepository()->LoadAnimation(modelResource, "asset/Model/player_dual_pistols_rapid_fire_lower.anim.fbx");
        Engine::ModelRepository()->LoadAnimation(modelResource, "asset/Model/player_dual_pistols_rapid_fire_upper.anim.fbx");
        Engine::ModelRepository()->LoadAnimation(modelResource, "asset/Model/player_dual_pistols_rapid_fire_walk_forward.anim.fbx");
        Engine::ModelRepository()->LoadAnimation(modelResource, "asset/Model/player_dual_pistols_rapid_fire_walk_back.anim.fbx");
        Engine::ModelRepository()->LoadAnimation(modelResource, "asset/Model/player_dual_pistols_rapid_fire_walk_right.anim.fbx");
        Engine::ModelRepository()->LoadAnimation(modelResource, "asset/Model/player_dual_pistols_rapid_fire_walk_left.anim.fbx");
    }

    // 再生設定を定義して、アニメーションクリップを登録
    {
        PlayOptions idle;
        idle.priority = static_cast<int>(Priority::Locomotion);
        idle.restart = false;
        idle.transitionTime = 0.15f;
        RegisterClip(Animation::Idle, FindClipIndex(modelComponent, "player_idle.anim.fbx"), SubMachine::Default, idle);

        PlayOptions running = idle;
        running.speed = 2.0f;
        running.restart = false;
        running.transitionTime = 0.15f;
        RegisterClip(Animation::Running, FindClipIndex(modelComponent, "player_running.anim.fbx"), SubMachine::Default, running);

        PlayOptions jump;
        jump.priority = static_cast<int>(Priority::Airborne);
        jump.loop = false;
        jump.waitForCompletion = true;
        jump.interruptible = true;
        m_jumpClips[0] = FindClipIndex(modelComponent, "player_jump_1.anim.fbx");
        m_jumpClips[1] = FindClipIndex(modelComponent, "player_jump_2.anim.fbx");
        RegisterClip(Animation::Jump, m_jumpClips[0], SubMachine::Default, jump);

        PlayOptions falling;
        falling.priority = static_cast<int>(Priority::Airborne);
        RegisterClip(Animation::Falling, FindClipIndex(modelComponent, "player_jump_2.anim.fbx"), SubMachine::Default, falling);

        PlayOptions shotgunIdle;
        shotgunIdle.priority = static_cast<int>(Priority::Weapon);
        shotgunIdle.transitionTime = 0.15f;
        shotgunIdle.speed = 2.0f;
        RegisterBlendTree2D(
            Animation::ShotgunAiming,
            {
                { FindClipIndex(modelComponent, "player_shotgun_idle.anim.fbx"), { 0.0f, 0.0f } },
                { FindClipIndex(modelComponent, "player_shotgun_walk_forward.anim.fbx"), { 0.0f, 1.0f } },
                { FindClipIndex(modelComponent, "player_shotgun_walk_back.anim.fbx"), { 0.0f, -1.0f } },
                { FindClipIndex(modelComponent, "player_shotgun_walk_right.anim.fbx"), { 1.0f, 0.0f } },
                { FindClipIndex(modelComponent, "player_shotgun_walk_left.anim.fbx"), { -1.0f, 0.0f } },
            },
            SubMachine::Shotgun,
            shotgunIdle);

        if (modelComponent && modelComponent->GetModelResource()) {
            RegisterLayerBlendTree1D(
                AnimationLayer::ShotgunAimVertical,
                {
                    { FindClipIndex(modelComponent, "player_shotgun_idle_lower.anim.fbx"), -1.0f },
                    { FindClipIndex(modelComponent, "player_shotgun_idle.anim.fbx"), 0.0f },
                    { FindClipIndex(modelComponent, "player_shotgun_idle_upper.anim.fbx"), 1.0f },
                },
                CreateUpperBodyMask(*modelComponent->GetModelResource()),
                SubMachine::Shotgun,
                shotgunIdle);
        }

        PlayOptions dualPistolsRapidFire;
        dualPistolsRapidFire.priority = static_cast<int>(Priority::Weapon);
        dualPistolsRapidFire.loop = true;
        dualPistolsRapidFire.transitionTime = 0.15f;
        dualPistolsRapidFire.speed = 2.0f;
        RegisterBlendTree2D(
            Animation::DualPistolsRapidFire,
            {
                { FindClipIndex(modelComponent, "player_dual_pistols_rapid_fire.anim.fbx"), { 0.0f, 0.0f } },
                { FindClipIndex(modelComponent, "player_dual_pistols_rapid_fire_walk_forward.anim.fbx"), { 0.0f, 1.0f } },
                { FindClipIndex(modelComponent, "player_dual_pistols_rapid_fire_walk_back.anim.fbx"), { 0.0f, -1.0f } },
                { FindClipIndex(modelComponent, "player_dual_pistols_rapid_fire_walk_right.anim.fbx"), { 1.0f, 0.0f } },
                { FindClipIndex(modelComponent, "player_dual_pistols_rapid_fire_walk_left.anim.fbx"), { -1.0f, 0.0f } },
            },
            SubMachine::DualPistols,
            dualPistolsRapidFire);

        if (modelResource) {
            dualPistolsRapidFire.speed = 3.0f;
            RegisterLayerBlendTree1D(
                AnimationLayer::DualPistolsRapidFireVertical,
                {
                    { FindClipIndex(modelComponent, "player_dual_pistols_rapid_fire_lower.anim.fbx"), -1.0f },
                    { FindClipIndex(modelComponent, "player_dual_pistols_rapid_fire.anim.fbx"), 0.0f },
                    { FindClipIndex(modelComponent, "player_dual_pistols_rapid_fire_upper.anim.fbx"), 1.0f },
                },
                CreateUpperBodyMask(*modelResource),
                SubMachine::DualPistols,
                dualPistolsRapidFire);
        }
    }
}

void PlayerAnimationController::BeginFrame()
{
    // 前フレームのリクエストをクリア
    m_frameRequests.clear();
    m_frameLayerRequests.clear();
}

void PlayerAnimationController::Update()
{
    if (!m_animationComponent) return;

    UpdateBaseAnimation();
    UpdateAnimationLayers();
}

void PlayerAnimationController::UpdateBaseAnimation()
{

    if (m_waitingForCompletion && m_animationComponent->IsFinished()) {
        m_waitingForCompletion = false;
        m_hasCurrentRequest = false;
    }

    const Request* next = FindHighestPriorityRequest();
    if (next && CanAccept(*next)) {
        const bool onlyUpdatingBlendTreeParameter = 
            m_hasCurrentRequest &&
            next->animation == m_currentRequest.animation &&
            (GetDefinitionType(next->animation) == DefinitionType::BlendTree1D ||
             GetDefinitionType(next->animation) == DefinitionType::BlendTree2D) &&
            !next->options.restart;
        if (!onlyUpdatingBlendTreeParameter) {
            HandleAnimationEvent(next->animation);
        }
        Apply(*next);
    }
}

#pragma region アニメーション再生
void PlayerAnimationController::PlayAnimation(Animation animation)
{
    const size_t index = static_cast<size_t>(animation);
    if (index >= m_clipDefinitions.size()) return;
    if (m_definitionTypes[index] != DefinitionType::Clip) return;
    PlayAnimation(animation, m_clipDefinitions[index].defaults);
}

void PlayerAnimationController::PlayAnimation(Animation animation, const PlayOptions& options)
{
    const size_t index = static_cast<size_t>(animation);
    if (index >= m_clipDefinitions.size()) return;
    if (m_definitionTypes[index] != DefinitionType::Clip) return;
    if (m_clipDefinitions[index].clipIndex < 0) return;

    Request request;
    request.animation = animation;
    request.options = options;
    request.order = m_requestOrder++;
    m_frameRequests.push_back(request);
}

void PlayerAnimationController::PlayBlendTree1D(Animation animation, float parameter)
{
    const size_t index = static_cast<size_t>(animation);
    if (index >= m_blendTree1DDefinitions.size()) return;
    if (m_definitionTypes[index] != DefinitionType::BlendTree1D) return;
    PlayBlendTree1D(animation, parameter, m_blendTree1DDefinitions[index].defaults);
}

void PlayerAnimationController::PlayBlendTree1D(
    Animation animation,
    float parameter,
    const PlayOptions& options)
{
    const size_t index = static_cast<size_t>(animation);
    if (index >= m_blendTree1DDefinitions.size()) return;
    if (m_definitionTypes[index] != DefinitionType::BlendTree1D) return;
    if (m_blendTree1DDefinitions[index].nodes.empty()) return;

    Request request;
    request.animation = animation;
    request.options = options;
    request.blendTree1DParameter = parameter;
    request.order = m_requestOrder++;
    m_frameRequests.push_back(request);
}

void PlayerAnimationController::PlayBlendTree2D(
    Animation animation,
    const DirectX::XMFLOAT2& parameter)
{
    const size_t index = static_cast<size_t>(animation);
    if (index >= m_blendTree2DDefinitions.size()) return;
    if (m_definitionTypes[index] != DefinitionType::BlendTree2D) return;
    PlayBlendTree2D(animation, parameter, m_blendTree2DDefinitions[index].defaults);
}

void PlayerAnimationController::PlayBlendTree2D(
    Animation animation,
    const DirectX::XMFLOAT2& parameter,
    const PlayOptions& options)
{
    const size_t index = static_cast<size_t>(animation);
    if (index >= m_blendTree2DDefinitions.size()) return;
    if (m_definitionTypes[index] != DefinitionType::BlendTree2D) return;
    if (m_blendTree2DDefinitions[index].nodes.empty()) return;

    Request request;
    request.animation = animation;
    request.options = options;
    request.blendTree2DParameter = parameter;
    request.order = m_requestOrder++;
    m_frameRequests.push_back(request);
}
#pragma endregion

#pragma region アニメーションレイヤー再生
void PlayerAnimationController::PlayLayerAnimation(AnimationLayer layer, float weight)
{
    const size_t index = static_cast<size_t>(layer);
    if (index >= m_layerDefinitions.size()) return;
    PlayLayerAnimation(layer, weight, m_layerDefinitions[index].defaults);
}

void PlayerAnimationController::PlayLayerAnimation(
    AnimationLayer layer,
    float weight,
    const PlayOptions& options)
{
    const size_t index = static_cast<size_t>(layer);
    if (index >= m_layerDefinitions.size()) return;
    if (m_layerDefinitions[index].type != DefinitionType::Clip) return;
    if (m_layerDefinitions[index].clipIndex < 0) return;
    m_frameLayerRequests.push_back({ layer, options, 0.0f, {}, weight, m_requestOrder++ });
}

void PlayerAnimationController::PlayLayerBlendTree1D(
    AnimationLayer layer,
    float parameter,
    float weight)
{
    const size_t index = static_cast<size_t>(layer);
    if (index >= m_layerDefinitions.size()) return;
    PlayLayerBlendTree1D(layer, parameter, weight, m_layerDefinitions[index].defaults);
}

void PlayerAnimationController::PlayLayerBlendTree1D(
    AnimationLayer layer,
    float parameter,
    float weight,
    const PlayOptions& options)
{
    const size_t index = static_cast<size_t>(layer);
    if (index >= m_layerDefinitions.size()) return;
    if (m_layerDefinitions[index].type != DefinitionType::BlendTree1D) return;
    if (m_layerDefinitions[index].blendTree1DNodes.empty()) return;
    m_frameLayerRequests.push_back({ layer, options, parameter, {}, weight, m_requestOrder++ });
}

void PlayerAnimationController::PlayLayerBlendTree2D(
    AnimationLayer layer,
    const DirectX::XMFLOAT2& parameter,
    float weight)
{
    const size_t index = static_cast<size_t>(layer);
    if (index >= m_layerDefinitions.size()) return;
    PlayLayerBlendTree2D(layer, parameter, weight, m_layerDefinitions[index].defaults);
}

void PlayerAnimationController::PlayLayerBlendTree2D(
    AnimationLayer layer,
    const DirectX::XMFLOAT2& parameter,
    float weight,
    const PlayOptions& options)
{
    const size_t index = static_cast<size_t>(layer);
    if (index >= m_layerDefinitions.size()) return;
    if (m_layerDefinitions[index].type != DefinitionType::BlendTree2D) return;
    if (m_layerDefinitions[index].blendTree2DNodes.empty()) return;
    m_frameLayerRequests.push_back({ layer, options, 0.0f, parameter, weight, m_requestOrder++ });
}
#pragma endregion

/// @brief 指定されたアニメーションに対応するアニメーションクリップのインデックスと再生設定を登録する
void PlayerAnimationController::RegisterClip(
    Animation animation,
    int clipIndex,
    SubMachine subMachine,
    const PlayOptions& defaults)
{
    const size_t index = static_cast<size_t>(animation);
    if (index >= m_clipDefinitions.size()) return;
    m_clipDefinitions[index] = { clipIndex, subMachine, defaults };
    m_blendTree1DDefinitions[index] = {};
    m_blendTree2DDefinitions[index] = {};
    m_definitionTypes[index] = DefinitionType::Clip;
}

void PlayerAnimationController::RegisterBlendTree1D(
    Animation animation,
    std::vector<AnimationBlendTree1DNode> nodes,
    SubMachine subMachine,
    const PlayOptions& defaults)
{
    const size_t index = static_cast<size_t>(animation);
    if (index >= m_blendTree1DDefinitions.size()) return;

    std::sort(nodes.begin(), nodes.end(),
        [](const AnimationBlendTree1DNode& lhs, const AnimationBlendTree1DNode& rhs) {
            return lhs.threshold < rhs.threshold;
        });

    m_clipDefinitions[index] = {};
    m_blendTree2DDefinitions[index] = {};
    m_blendTree1DDefinitions[index] = { std::move(nodes), subMachine, defaults };
    m_definitionTypes[index] = DefinitionType::BlendTree1D;
}

void PlayerAnimationController::RegisterBlendTree2D(
    Animation animation,
    std::vector<AnimationBlendTree2DNode> nodes,
    SubMachine subMachine,
    const PlayOptions& defaults)
{
    const size_t index = static_cast<size_t>(animation);
    if (index >= m_blendTree2DDefinitions.size()) return;
    m_clipDefinitions[index] = {};
    m_blendTree1DDefinitions[index] = {};
    m_blendTree2DDefinitions[index] = { std::move(nodes), subMachine, defaults };
    m_definitionTypes[index] = DefinitionType::BlendTree2D;
}

void PlayerAnimationController::RegisterLayerClip(
    AnimationLayer layer,
    int clipIndex,
    const AnimationBoneMask& mask,
    SubMachine subMachine,
    const PlayOptions& defaults,
    float defaultWeight)
{
    if (!m_animationComponent) return;
    const size_t index = static_cast<size_t>(layer);
    if (index >= m_layerDefinitions.size()) return;
    const size_t componentLayerIndex = m_animationComponent->AddAnimationLayer(mask, defaultWeight);
    m_animationComponent->StopAnimationLayer(componentLayerIndex);
    LayerDefinition& definition = m_layerDefinitions[index];
    definition.type = DefinitionType::Clip;
    definition.componentLayerIndex = componentLayerIndex;
    definition.clipIndex = clipIndex;
    definition.subMachine = subMachine;
    definition.defaults = defaults;
    definition.defaultWeight = defaultWeight;
}

void PlayerAnimationController::RegisterLayerBlendTree1D(
    AnimationLayer layer,
    std::vector<AnimationBlendTree1DNode> nodes,
    const AnimationBoneMask& mask,
    SubMachine subMachine,
    const PlayOptions& defaults,
    float defaultWeight)
{
    if (!m_animationComponent) return;
    const size_t index = static_cast<size_t>(layer);
    if (index >= m_layerDefinitions.size()) return;
    std::sort(nodes.begin(), nodes.end(),
        [](const AnimationBlendTree1DNode& lhs, const AnimationBlendTree1DNode& rhs) {
            return lhs.threshold < rhs.threshold;
        });
    const size_t componentLayerIndex = m_animationComponent->AddAnimationLayer(mask, defaultWeight);
    m_animationComponent->StopAnimationLayer(componentLayerIndex);
    LayerDefinition& definition = m_layerDefinitions[index];
    definition.type = DefinitionType::BlendTree1D;
    definition.componentLayerIndex = componentLayerIndex;
    definition.blendTree1DNodes = std::move(nodes);
    definition.subMachine = subMachine;
    definition.defaults = defaults;
    definition.defaultWeight = defaultWeight;
}

void PlayerAnimationController::RegisterLayerBlendTree2D(
    AnimationLayer layer,
    std::vector<AnimationBlendTree2DNode> nodes,
    const AnimationBoneMask& mask,
    SubMachine subMachine,
    const PlayOptions& defaults,
    float defaultWeight)
{
    if (!m_animationComponent) return;
    const size_t index = static_cast<size_t>(layer);
    if (index >= m_layerDefinitions.size()) return;
    const size_t componentLayerIndex = m_animationComponent->AddAnimationLayer(mask, defaultWeight);
    m_animationComponent->StopAnimationLayer(componentLayerIndex);
    LayerDefinition& definition = m_layerDefinitions[index];
    definition.type = DefinitionType::BlendTree2D;
    definition.componentLayerIndex = componentLayerIndex;
    definition.blendTree2DNodes = std::move(nodes);
    definition.subMachine = subMachine;
    definition.defaults = defaults;
    definition.defaultWeight = defaultWeight;
}

void PlayerAnimationController::ChangeClip(Animation animation, int clipIndex)
{
    const size_t index = static_cast<size_t>(animation);
    if (index >= m_clipDefinitions.size()) return;
    if (m_definitionTypes[index] != DefinitionType::Clip) return;
    m_clipDefinitions[index].clipIndex = clipIndex;
}

#pragma region サブマシーンの管理
bool PlayerAnimationController::EnterSubMachine(SubMachine subMachine)
{
    if (subMachine == SubMachine::Any || subMachine == SubMachine::MAX) return false;
    if (subMachine == m_currentSubMachine) return true;
    if (m_currentSubMachine != SubMachine::Default) return false;

    ForceSetSubMachine(subMachine);
    return true;
}

void PlayerAnimationController::RequestExitSubMachine()
{
    if (m_currentSubMachine == SubMachine::Default) return;
    ForceSetSubMachine(SubMachine::Default);
}

void PlayerAnimationController::ForceSetSubMachine(SubMachine subMachine)
{
    if (subMachine == SubMachine::Any || subMachine == SubMachine::MAX) return;

    m_currentSubMachine = subMachine;
    m_hasCurrentRequest = false;
    m_waitingForCompletion = false;
}
#pragma endregion

/// @brief 指定されたアニメーションのデフォルト再生設定を取得する
PlayerAnimationController::PlayOptions PlayerAnimationController::GetDefaultPlayOptions(Animation animation) const
{
    DefinitionType type = GetDefinitionType(animation);

    switch (type) {
        case DefinitionType::Clip:
            return m_clipDefinitions[static_cast<size_t>(animation)].defaults;
        case DefinitionType::BlendTree1D:
            return m_blendTree1DDefinitions[static_cast<size_t>(animation)].defaults;
        case DefinitionType::BlendTree2D:
            return m_blendTree2DDefinitions[static_cast<size_t>(animation)].defaults;
        default: 
            return PlayerAnimationController::PlayOptions{};
    }
}

PlayerAnimationController::PlayOptions PlayerAnimationController::GetDefaultPlayOptions(AnimationLayer layer) const
{
    return m_layerDefinitions[static_cast<size_t>(layer)].defaults;
}

/// @brief 現在のフレームで最も優先度の高いアニメーションリクエストを検索する
const PlayerAnimationController::Request* PlayerAnimationController::FindHighestPriorityRequest() const
{
    const Request* result = nullptr;
    for (const Request& request : m_frameRequests) {
        if (!IsAvailableInCurrentSubMachine(request.animation)) continue;
        if (!result) {
            result = &request;
            continue;
        }

        if (request.options.priority != result->options.priority) {
            if (request.options.priority > result->options.priority) result = &request;
            continue;
        }

        const bool requestIsCurrent =
            m_hasCurrentRequest && request.animation == m_currentRequest.animation;
        const bool resultIsCurrent =
            m_hasCurrentRequest && result->animation == m_currentRequest.animation;
        if (requestIsCurrent != resultIsCurrent) {
            if (requestIsCurrent) result = &request;
            continue;
        }

        if (request.order < result->order) result = &request;
    }
    return result;
}

const PlayerAnimationController::LayerRequest*
PlayerAnimationController::FindHighestPriorityLayerRequest(AnimationLayer layer) const
{
    const LayerRequest* result = nullptr;
    for (const LayerRequest& request : m_frameLayerRequests) {
        if (request.layer != layer) continue;
        if (!result ||
            request.options.priority > result->options.priority ||
            (request.options.priority == result->options.priority && request.order < result->order)) {
            result = &request;
        }
    }
    return result;
}

bool PlayerAnimationController::IsAvailableInCurrentSubMachine(Animation animation) const
{
    const size_t index = static_cast<size_t>(animation);
    if (index >= m_definitionTypes.size()) return false;

    SubMachine owner = SubMachine::Default;
    switch (m_definitionTypes[index]) {
    case DefinitionType::Clip:
        owner = m_clipDefinitions[index].subMachine;
        break;
    case DefinitionType::BlendTree1D:
        owner = m_blendTree1DDefinitions[index].subMachine;
        break;
    case DefinitionType::BlendTree2D:
        owner = m_blendTree2DDefinitions[index].subMachine;
        break;
    default:
        return false;
    }
    return owner == SubMachine::Any || owner == m_currentSubMachine;
}

PlayerAnimationController::DefinitionType PlayerAnimationController::GetDefinitionType(
    Animation animation) const
{
    const size_t index = static_cast<size_t>(animation);
    if (index >= m_definitionTypes.size()) return DefinitionType::None;
    return m_definitionTypes[index];
}

/// @brief 指定されたアニメーションリクエストを受け入れ可能かどうかを判定する
bool PlayerAnimationController::CanAccept(const Request& request) const
{
    // 1. 現在のアニメーションがない場合は、どのリクエストも受け入れ可能
    if (!m_hasCurrentRequest) return true;
    // 2. 同じBlendTreeの場合、再スタートせずParameterを更新するため受け付ける
    if (request.animation == m_currentRequest.animation) {
        const DefinitionType type = GetDefinitionType(request.animation);
        if (type == DefinitionType::BlendTree1D || type == DefinitionType::BlendTree2D) return true;
        return request.options.restart;
    }
    // 3. 完了待ちでない場合は
    if (!m_waitingForCompletion) return true;
    // 4. 新しいリクエストが強制割り込みの場合
    if (request.options.forceInterrupt) return true;
    // 5. 現在のアニメーションが割り込み可能かどうか
    if (!m_currentRequest.options.interruptible) return false;
    // 6. 新しいリクエストの優先度が高い場合
    return request.options.priority > m_currentRequest.options.priority;
}

/// @brief 指定されたアニメーションリクエストを適用して再生する
void PlayerAnimationController::Apply(const Request& request)
{
    const size_t index = static_cast<size_t>(request.animation);
    switch (m_definitionTypes[index]) {
    case DefinitionType::Clip: {
        m_animationComponent->PlayAnimation(
            m_clipDefinitions[index].clipIndex,
            request.options.speed,
            request.options.loop,
            request.options.restart,
            request.options.transitionTime);
        break;
    }
    case DefinitionType::BlendTree1D: {
        m_animationComponent->PlayBlendTree1D(
            m_blendTree1DDefinitions[index].nodes,
            request.blendTree1DParameter,
            request.options.speed,
            request.options.loop,
            request.options.restart);
        break;
    }
    case DefinitionType::BlendTree2D: {
        m_animationComponent->PlayBlendTree2D(
            m_blendTree2DDefinitions[index].nodes,
            request.blendTree2DParameter,
            request.options.speed,
            request.options.loop,
            request.options.restart);
        break;
    }
    default:
        return;
    }

    m_currentRequest = request;
    m_hasCurrentRequest = true;
    m_waitingForCompletion = request.options.waitForCompletion;
}

void PlayerAnimationController::UpdateAnimationLayers()
{
    for (size_t i = 0; i < m_layerDefinitions.size(); ++i) {
        const LayerDefinition& definition = m_layerDefinitions[i];
        if (definition.type == DefinitionType::None) continue;

        const AnimationLayer layer = static_cast<AnimationLayer>(i);
        const LayerRequest* request = FindHighestPriorityLayerRequest(layer);
        const bool available =
            definition.subMachine == SubMachine::Any ||
            definition.subMachine == m_currentSubMachine;
        if (!request || !available) {
            m_animationComponent->StopAnimationLayer(definition.componentLayerIndex);
            continue;
        }
        ApplyLayer(*request);
    }
}

void PlayerAnimationController::ApplyLayer(const LayerRequest& request)
{
    const size_t index = static_cast<size_t>(request.layer);
    if (index >= m_layerDefinitions.size()) return;
    const LayerDefinition& definition = m_layerDefinitions[index];
    m_animationComponent->SetAnimationLayerWeight(definition.componentLayerIndex, request.weight);

    switch (definition.type) {
    case DefinitionType::Clip:
        m_animationComponent->PlayLayerAnimation(
            definition.componentLayerIndex, definition.clipIndex,
            request.options.speed, request.options.loop, request.options.restart);
        break;
    case DefinitionType::BlendTree1D:
        m_animationComponent->PlayLayerBlendTree1D(
            definition.componentLayerIndex, definition.blendTree1DNodes,
            request.blendTree1DParameter,
            request.options.speed, request.options.loop, request.options.restart);
        break;
    case DefinitionType::BlendTree2D:
        m_animationComponent->PlayLayerBlendTree2D(
            definition.componentLayerIndex, definition.blendTree2DNodes,
            request.blendTree2DParameter,
            request.options.speed, request.options.loop, request.options.restart);
        break;
    default:
        break;
    }
}

/// @brief 指定されたアニメーションが設定された瞬間に呼び出される
void PlayerAnimationController::HandleAnimationEvent(Animation animation)
{
    switch (animation) {
    case Animation::Jump: 
    {
        m_jumpFlipCount = (m_jumpFlipCount + 1) % 2;
        ChangeClip(Animation::Jump, m_jumpClips[m_jumpFlipCount]);
        break;
    }
    default: break;
    }
}
