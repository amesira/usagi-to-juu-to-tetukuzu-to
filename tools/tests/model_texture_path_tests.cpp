#include "Engine/Graphics/model_texture_path.h"
#include <cassert>
#include <iostream>
int main() {
    using ModelTexturePath::ResolveExternal;
    assert(ResolveExternal("asset/Model/sky_sphere.fbx", "sky_texture.png") == std::filesystem::path("asset/Model/sky_texture.png"));
    assert(ResolveExternal("asset/Model/sky_sphere.fbx", "../Texture/sky.png") == std::filesystem::path("asset/Texture/sky.png"));
    assert(ResolveExternal("asset/Model/sky_sphere.fbx", "textures/./sky.png") == std::filesystem::path("asset/Model/textures/sky.png"));
    assert(ResolveExternal("sky.fbx", "sky.png") == std::filesystem::path("sky.png"));
    assert(ResolveExternal("asset/Model/sky.fbx", "").empty());
    assert(ResolveExternal("asset/Model/sky.fbx", "*0").empty());
    assert(ResolveExternal("asset/Model/sky.fbx", "C:\\Textures\\sky.png") == std::filesystem::path("C:/Textures/sky.png"));
    assert(ResolveExternal("C:/Game/Models/sky.fbx", "..\\Textures\\sky.png") == std::filesystem::path("C:/Game/Textures/sky.png"));
    assert(ResolveExternal("C:/Game/Models/sky.fbx", "\\\\server\\share\\sky.png") == std::filesystem::path("\\\\server\\share\\sky.png"));
    const std::string unicode = "テクスチャ/空.png";
    const auto expected = std::filesystem::path(u8"asset/Model/テクスチャ/空.png");
    assert(ResolveExternal("asset/Model/sky.fbx", unicode) == expected);
    std::cout << "Model texture path tests passed\n";
}
