#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <iostream>
int main() {
    for (const char* path : {"asset/Model/enemy_a_model.fbx", "asset/Model/enemy_b_model.fbx"}) {
        Assimp::Importer importer;
        const auto* scene = importer.ReadFile(path, aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_ConvertToLeftHanded | aiProcess_Triangulate);
        if (!scene) { std::cerr << importer.GetErrorString(); return 1; }
        std::cout << path << '\n';
        for (unsigned i = 0; i < scene->mNumMaterials; ++i) std::cout << i << ": " << scene->mMaterials[i]->GetName().C_Str() << '\n';
    }
}
