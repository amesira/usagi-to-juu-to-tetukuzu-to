//---------------------------------------------------
// model_animation_utility.h
// ・モデルのボーンTransform取得に関連する共通処理
//---------------------------------------------------
#ifndef MODEL_ANIMATION_UTILITY_H
#define MODEL_ANIMATION_UTILITY_H

#include <string>

#include "Engine/Device/direct3d.h"

class ModelComponent;
class TransformComponent;

namespace ModelAnimationUtility {

    struct BoneTransform {
        DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
        DirectX::XMFLOAT4 rotation = { 0.0f, 0.0f, 0.0f, 1.0f };
        DirectX::XMFLOAT3 scale = { 1.0f, 1.0f, 1.0f };
    };

    // ボーン名からインデックスを取得する。毎フレーム使う場合は結果をキャッシュすること。
    bool FindBoneIndex(
        const ModelComponent& model,
        const std::string& boneName,
        unsigned int& outBoneIndex);

    // モデル空間におけるボーン行列を取得する。
    bool GetBoneModelMatrix(
        const ModelComponent& model,
        unsigned int boneIndex,
        DirectX::XMMATRIX& outMatrix);
    bool GetBoneModelMatrix(
        const ModelComponent& model,
        const std::string& boneName,
        DirectX::XMMATRIX& outMatrix);

    // ワールド空間におけるボーン行列を取得する。
    bool GetBoneWorldMatrix(
        const ModelComponent& model,
        const TransformComponent& modelTransform,
        unsigned int boneIndex,
        DirectX::XMMATRIX& outMatrix);
    bool GetBoneWorldMatrix(
        const ModelComponent& model,
        const TransformComponent& modelTransform,
        const std::string& boneName,
        DirectX::XMMATRIX& outMatrix);

    // 行列を位置・回転・スケールへ分解する。
    bool DecomposeMatrix(
        const DirectX::XMMATRIX& matrix,
        BoneTransform& outTransform);

    bool GetBoneModelTransform(
        const ModelComponent& model,
        unsigned int boneIndex,
        BoneTransform& outTransform);
    bool GetBoneModelTransform(
        const ModelComponent& model,
        const std::string& boneName,
        BoneTransform& outTransform);

    bool GetBoneWorldTransform(
        const ModelComponent& model,
        const TransformComponent& modelTransform,
        unsigned int boneIndex,
        BoneTransform& outTransform);
    bool GetBoneWorldTransform(
        const ModelComponent& model,
        const TransformComponent& modelTransform,
        const std::string& boneName,
        BoneTransform& outTransform);

}

#endif // MODEL_ANIMATION_UTILITY_H
