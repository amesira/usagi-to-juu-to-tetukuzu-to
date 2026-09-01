//===================================================
// File  ：_/DualPistols/player_dual_pistols_effects.cpp
// Date  ：2026/08/28
// Author：Miu Kitamura
//===================================================
#include "player_dual_pistols_effects.h"
#include "player_dual_pistols_context.h"

#include "Engine/Component/particle_system_component.h"
#include "Engine/Component/light_component.h"
#include "Engine/Component/mesh_effect_component.h"
#include "Engine/Component/transform_component.h"

#include "Game/Factory/render_effect_factory.h"

#include "Game/ControllerBehavior/game_controller_locator.h"
#include "Game/ControllerBehavior/custom_post_effect_controller.h"
#include "Game/ControllerBehavior/game_feedback_controller.h"

#include <filesystem>

namespace {
    const std::filesystem::path MUZZLE_FLASH_EFFECT_ASSET =
        "asset/Particle/player_shotgun_fire.particle.json";
    const std::filesystem::path SLASH_BURST_EFFECT_ASSETS[3] = {
        "asset/MeshEffect/player_slash_burst_1_effect.mesh_effect.json",
        "asset/MeshEffect/player_slash_burst_1_effect.mesh_effect.json",
        "asset/MeshEffect/player_slash_burst_1_effect.mesh_effect.json",
    };
}

void PlayerDualPistolsEffects::Initialize(PlayerDualPistolsContext& context)
{

}

void PlayerDualPistolsEffects::Finalize()
{

}

void PlayerDualPistolsEffects::PlayEffects(PlayerDualPistolsContext& context, EffectsType effectType)
{

}
