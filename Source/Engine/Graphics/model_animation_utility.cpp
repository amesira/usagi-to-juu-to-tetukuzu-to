//===================================================
// model_animation_utility.cpp
//===================================================
#include "model_animation_utility.h"

#include "Engine/Component/model_component.h"
#include "Engine/Component/transform_component.h"

namespace ModelAnimationUtility {

    bool FindBoneIndex(
        const ModelComponent& model,
        const std::string& boneName,
        unsigned int& outBoneIndex)
    {
        const ModelResource* resource = model.GetModelResource();
        if (!resource) return false;

        const auto it = resource->boneNameToIndex.find(boneName);
        if (it == resource->boneNameToIndex.end()) return false;

        outBoneIndex = it->second;
        return true;
    }

    bool GetBoneModelMatrix(
        const ModelComponent& model,
        unsigned int boneIndex,
        DirectX::XMMATRIX& outMatrix)
    {
        const ModelResource* resource = model.GetModelResource();
        if (!resource || boneIndex >= resource->bones.size()) return false;

        const SkeletonPose& pose = model.GetSkeletonPose();
        if (boneIndex >= pose.globalTransforms.size()) return false;

        outMatrix = pose.globalTransforms[boneIndex];
        return true;
    }

    bool GetBoneModelMatrix(
        const ModelComponent& model,
        const std::string& boneName,
        DirectX::XMMATRIX& outMatrix)
    {
        unsigned int boneIndex = 0;
        return FindBoneIndex(model, boneName, boneIndex)
            && GetBoneModelMatrix(model, boneIndex, outMatrix);
    }

    bool GetBoneWorldMatrix(
        const ModelComponent& model,
        const TransformComponent& modelTransform,
        unsigned int boneIndex,
        DirectX::XMMATRIX& outMatrix)
    {
        DirectX::XMMATRIX boneModelMatrix;
        if (!GetBoneModelMatrix(model, boneIndex, boneModelMatrix)) return false;

        outMatrix = boneModelMatrix * modelTransform.GetWorldMatrix();
        return true;
    }

    bool GetBoneWorldMatrix(
        const ModelComponent& model,
        const TransformComponent& modelTransform,
        const std::string& boneName,
        DirectX::XMMATRIX& outMatrix)
    {
        unsigned int boneIndex = 0;
        return FindBoneIndex(model, boneName, boneIndex)
            && GetBoneWorldMatrix(model, modelTransform, boneIndex, outMatrix);
    }

    bool DecomposeMatrix(
        const DirectX::XMMATRIX& matrix,
        BoneTransform& outTransform)
    {
        using namespace DirectX;

        XMVECTOR scale;
        XMVECTOR rotation;
        XMVECTOR translation;
        if (!XMMatrixDecompose(&scale, &rotation, &translation, matrix)) return false;

        XMStoreFloat3(&outTransform.position, translation);
        XMStoreFloat4(&outTransform.rotation, XMQuaternionNormalize(rotation));
        XMStoreFloat3(&outTransform.scale, scale);
        return true;
    }

    bool GetBoneModelTransform(
        const ModelComponent& model,
        unsigned int boneIndex,
        BoneTransform& outTransform)
    {
        DirectX::XMMATRIX matrix;
        return GetBoneModelMatrix(model, boneIndex, matrix)
            && DecomposeMatrix(matrix, outTransform);
    }

    bool GetBoneModelTransform(
        const ModelComponent& model,
        const std::string& boneName,
        BoneTransform& outTransform)
    {
        unsigned int boneIndex = 0;
        return FindBoneIndex(model, boneName, boneIndex)
            && GetBoneModelTransform(model, boneIndex, outTransform);
    }

    bool GetBoneWorldTransform(
        const ModelComponent& model,
        const TransformComponent& modelTransform,
        unsigned int boneIndex,
        BoneTransform& outTransform)
    {
        DirectX::XMMATRIX matrix;
        return GetBoneWorldMatrix(model, modelTransform, boneIndex, matrix)
            && DecomposeMatrix(matrix, outTransform);
    }

    bool GetBoneWorldTransform(
        const ModelComponent& model,
        const TransformComponent& modelTransform,
        const std::string& boneName,
        BoneTransform& outTransform)
    {
        unsigned int boneIndex = 0;
        return FindBoneIndex(model, boneName, boneIndex)
            && GetBoneWorldTransform(model, modelTransform, boneIndex, outTransform);
    }

}
