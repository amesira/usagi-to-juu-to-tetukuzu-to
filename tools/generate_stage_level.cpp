#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <DirectXMath.h>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <set>
#include <string>
#include <vector>

using json = nlohmann::json;
using namespace DirectX;

namespace {
constexpr float kThickness = 0.15f;
constexpr float kCoplanarEpsilon = 0.01f;
constexpr float kNormalDotThreshold = 0.9995f;

struct Triangle {
    aiVector3D vertices[3];
    aiVector3D normal;
    unsigned indices[3];
    bool used = false;
};

struct Box {
    aiVector3D center;
    aiVector3D size;
    aiVector3D rotationDegrees;
};

aiVector3D Normalize(const aiVector3D& value) {
    const float length = value.Length();
    return length > 0.000001f ? value / length : aiVector3D{};
}

float DistanceSquared(const aiVector3D& a, const aiVector3D& b) {
    return (a - b).SquareLength();
}

std::pair<unsigned, unsigned> EdgeKey(unsigned a, unsigned b) {
    return a < b ? std::pair{a, b} : std::pair{b, a};
}

bool Coplanar(const Triangle& a, const Triangle& b) {
    if (std::abs(a.normal * b.normal) < kNormalDotThreshold) return false;
    return std::abs(a.normal * (b.vertices[0] - a.vertices[0])) <= kCoplanarEpsilon;
}

aiVector3D ToEulerDegrees(const aiVector3D& right, const aiVector3D& up, const aiVector3D& forward) {
    const XMMATRIX rotation = XMMatrixSet(
        right.x, right.y, right.z, 0.0f,
        up.x, up.y, up.z, 0.0f,
        forward.x, forward.y, forward.z, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);
    XMFLOAT4 q;
    XMStoreFloat4(&q, XMQuaternionNormalize(XMQuaternionRotationMatrix(rotation)));
    const float pitch = std::asin(std::clamp(2.0f * (q.w * q.x - q.y * q.z), -1.0f, 1.0f));
    const float yaw = std::atan2(2.0f * (q.w * q.y + q.x * q.z), 1.0f - 2.0f * (q.x * q.x + q.y * q.y));
    const float roll = std::atan2(2.0f * (q.w * q.z + q.x * q.y), 1.0f - 2.0f * (q.x * q.x + q.z * q.z));
    constexpr float radiansToDegrees = 180.0f / XM_PI;
    return {pitch * radiansToDegrees, yaw * radiansToDegrees, roll * radiansToDegrees};
}

aiVector3D ToEulerDegrees(const aiQuaternion& rotation) {
    aiQuaternion normalized = rotation;
    normalized.Normalize();
    const float pitch = std::asin(std::clamp(
        2.0f * (normalized.w * normalized.x - normalized.y * normalized.z), -1.0f, 1.0f));
    const float yaw = std::atan2(
        2.0f * (normalized.w * normalized.y + normalized.x * normalized.z),
        1.0f - 2.0f * (normalized.x * normalized.x + normalized.y * normalized.y));
    const float roll = std::atan2(
        2.0f * (normalized.w * normalized.z + normalized.x * normalized.y),
        1.0f - 2.0f * (normalized.x * normalized.x + normalized.z * normalized.z));
    constexpr float radiansToDegrees = 180.0f / XM_PI;
    return {pitch * radiansToDegrees, yaw * radiansToDegrees, roll * radiansToDegrees};
}

Box MakeBox(const std::vector<aiVector3D>& points, const aiVector3D& surfaceNormal) {
    aiVector3D normal = Normalize(surfaceNormal);

    // 各辺方向を候補に、投影矩形の面積が最小になる軸を使用する。
    aiVector3D axisRight{1.0f, 0.0f, 0.0f};
    float smallestArea = std::numeric_limits<float>::max();
    for (size_t i = 0; i < points.size(); ++i) {
        for (size_t j = i + 1; j < points.size(); ++j) {
            aiVector3D candidate = points[j] - points[i];
            candidate -= normal * (candidate * normal);
            const float lengthSquared = candidate.SquareLength();
            if (lengthSquared <= 0.000001f) continue;
            candidate = Normalize(candidate);
            const aiVector3D candidateForward = Normalize(candidate ^ normal);
            float candidateMinX = std::numeric_limits<float>::max(), candidateMaxX = -candidateMinX;
            float candidateMinZ = std::numeric_limits<float>::max(), candidateMaxZ = -candidateMinZ;
            for (const aiVector3D& point : points) {
                candidateMinX = std::min(candidateMinX, point * candidate);
                candidateMaxX = std::max(candidateMaxX, point * candidate);
                candidateMinZ = std::min(candidateMinZ, point * candidateForward);
                candidateMaxZ = std::max(candidateMaxZ, point * candidateForward);
            }
            const float area = (candidateMaxX - candidateMinX) * (candidateMaxZ - candidateMinZ);
            if (area < smallestArea) { smallestArea = area; axisRight = candidate; }
        }
    }
    aiVector3D forward = Normalize(axisRight ^ normal);
    axisRight = Normalize(normal ^ forward);

    float minX = std::numeric_limits<float>::max(), maxX = -minX;
    float minZ = std::numeric_limits<float>::max(), maxZ = -minZ;
    float plane = 0.0f;
    for (const aiVector3D& point : points) {
        minX = std::min(minX, point * axisRight); maxX = std::max(maxX, point * axisRight);
        minZ = std::min(minZ, point * forward); maxZ = std::max(maxZ, point * forward);
        plane += point * normal;
    }
    plane /= static_cast<float>(points.size());

    Box result;
    result.center = axisRight * ((minX + maxX) * 0.5f)
        + normal * plane
        + forward * ((minZ + maxZ) * 0.5f);
    result.size = {std::max(maxX - minX, kThickness), kThickness, std::max(maxZ - minZ, kThickness)};
    result.rotationDegrees = ToEulerDegrees(axisRight, normal, forward);
    return result;
}

std::vector<Triangle> ReadTriangles(const aiScene* scene) {
    std::vector<Triangle> triangles;
    const auto walk = [&](const auto& self, const aiNode* node, const aiMatrix4x4& parent) -> void {
        const aiMatrix4x4 world = parent * node->mTransformation;
        for (unsigned meshIndex = 0; meshIndex < node->mNumMeshes; ++meshIndex) {
            const aiMesh* mesh = scene->mMeshes[node->mMeshes[meshIndex]];
            for (unsigned faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
                const aiFace& face = mesh->mFaces[faceIndex];
                if (face.mNumIndices != 3) continue;
                Triangle triangle;
                for (unsigned i = 0; i < 3; ++i) {
                    triangle.indices[i] = face.mIndices[i];
                    triangle.vertices[i] = world * mesh->mVertices[face.mIndices[i]];
                }
                triangle.normal = Normalize((triangle.vertices[1] - triangle.vertices[0]) ^ (triangle.vertices[2] - triangle.vertices[0]));
                triangles.push_back(triangle);
            }
        }
        for (unsigned i = 0; i < node->mNumChildren; ++i) self(self, node->mChildren[i], world);
    };
    walk(walk, scene->mRootNode, aiMatrix4x4{});
    return triangles;
}

std::vector<Box> BuildBoxes(std::vector<Triangle>& triangles) {
    std::map<std::pair<unsigned, unsigned>, std::vector<size_t>> edgeFaces;
    for (size_t i = 0; i < triangles.size(); ++i)
        for (int edge = 0; edge < 3; ++edge)
            edgeFaces[EdgeKey(triangles[i].indices[edge], triangles[i].indices[(edge + 1) % 3])].push_back(i);

    std::vector<Box> boxes;
    for (size_t i = 0; i < triangles.size(); ++i) {
        if (triangles[i].used) continue;
        size_t partner = triangles.size();
        float longestSharedEdge = -1.0f;
        for (int edge = 0; edge < 3; ++edge) {
            const unsigned a = triangles[i].indices[edge];
            const unsigned b = triangles[i].indices[(edge + 1) % 3];
            for (size_t candidate : edgeFaces[EdgeKey(a, b)]) {
                if (candidate == i || triangles[candidate].used || !Coplanar(triangles[i], triangles[candidate])) continue;
                const float edgeLength = DistanceSquared(triangles[i].vertices[edge], triangles[i].vertices[(edge + 1) % 3]);
                if (edgeLength > longestSharedEdge) { longestSharedEdge = edgeLength; partner = candidate; }
            }
        }

        std::vector<aiVector3D> points(std::begin(triangles[i].vertices), std::end(triangles[i].vertices));
        triangles[i].used = true;
        if (partner < triangles.size()) {
            triangles[partner].used = true;
            for (const aiVector3D& point : triangles[partner].vertices) {
                const bool duplicate = std::any_of(points.begin(), points.end(), [&](const aiVector3D& existing) {
                    return DistanceSquared(point, existing) < 0.000001f;
                });
                if (!duplicate) points.push_back(point);
            }
        }
        boxes.push_back(MakeBox(points, triangles[i].normal));
    }
    return boxes;
}

std::vector<Box> BuildNodeBoxes(const aiScene* scene) {
    std::vector<Box> boxes;
    const auto walk = [&](const auto& self, const aiNode* node, const aiMatrix4x4& parent) -> void {
        const aiMatrix4x4 world = parent * node->mTransformation;
        aiVector3D worldScale;
        aiQuaternion worldRotation;
        aiVector3D worldPosition;
        world.Decompose(worldScale, worldRotation, worldPosition);

        for (unsigned nodeMeshIndex = 0; nodeMeshIndex < node->mNumMeshes; ++nodeMeshIndex) {
            const aiMesh* mesh = scene->mMeshes[node->mMeshes[nodeMeshIndex]];
            if (!mesh || mesh->mNumVertices == 0) continue;

            aiVector3D localMin(std::numeric_limits<float>::max());
            aiVector3D localMax(-std::numeric_limits<float>::max());
            for (unsigned vertexIndex = 0; vertexIndex < mesh->mNumVertices; ++vertexIndex) {
                const aiVector3D& point = mesh->mVertices[vertexIndex];
                localMin.x = std::min(localMin.x, point.x);
                localMin.y = std::min(localMin.y, point.y);
                localMin.z = std::min(localMin.z, point.z);
                localMax.x = std::max(localMax.x, point.x);
                localMax.y = std::max(localMax.y, point.y);
                localMax.z = std::max(localMax.z, point.z);
            }

            const aiVector3D localCenter = (localMin + localMax) * 0.5f;
            const aiVector3D worldCenter = world * localCenter;
            const aiVector3D localSize = localMax - localMin;

            Box box;
            box.center = worldCenter;
            box.size = {
                std::abs(localSize.x * worldScale.x),
                std::abs(localSize.y * worldScale.y),
                std::abs(localSize.z * worldScale.z)
            };
            box.rotationDegrees = ToEulerDegrees(worldRotation);
            boxes.push_back(box);

            std::cout << node->mName.C_Str()
                << " size=" << box.size.x << ',' << box.size.y << ',' << box.size.z
                << " position=" << box.center.x << ',' << box.center.y << ',' << box.center.z << '\n';
        }
        for (unsigned childIndex = 0; childIndex < node->mNumChildren; ++childIndex)
            self(self, node->mChildren[childIndex], world);
    };
    walk(walk, scene->mRootNode, aiMatrix4x4{});
    return boxes;
}

json Vec3(const aiVector3D& value) { return json::array({value.x, value.y, value.z}); }
}

int main(int argc, char** argv) {
    const std::filesystem::path input = argc > 1 ? argv[1] : "asset/Level/Stages.fbx";
    const std::filesystem::path output = argc > 2 ? argv[2] : "asset/Level/stage.level.json";
    const std::filesystem::path visualModel = argc > 3 ? argv[3] : "asset/Level/Stage.fbx";

    Assimp::Importer importer;
    // 描画用モデルと同じ座標系・後処理で読み込み、
    // 生成したColliderと描画メッシュの向きを一致させる。
    const aiScene* scene = importer.ReadFile(input.string(),
        aiProcessPreset_TargetRealtime_MaxQuality |
        aiProcess_ConvertToLeftHanded |
        aiProcess_Triangulate);
    if (!scene) { std::cerr << importer.GetErrorString() << '\n'; return 1; }

    std::vector<Box> boxes = BuildNodeBoxes(scene);

    json objects = json::array();
    objects.push_back({
        {"id", "stage_visual"}, {"name", "Stage"}, {"tag", "Untagged"},
        {"renderLayer", "Default"}, {"collisionLayer", "Default"},
        {"transform", {{"position", {0.0f,0.0f,0.0f}}, {"rotationDegrees", {0.0f,0.0f,0.0f}}, {"scale", {1.0f,1.0f,1.0f}}}},
        {"collider", {{"type", "None"}, {"center", {0.0f,0.0f,0.0f}}, {"boxSize", {1.0f,1.0f,1.0f}}, {"sphereRadius", 0.5f}}},
        {"modelPath", visualModel.generic_string()}, {"materialName", ""}
    });

    for (size_t i = 0; i < boxes.size(); ++i) {
        const Box& box = boxes[i];
        std::ostringstream id; id << "stage_collider_" << std::setw(4) << std::setfill('0') << i + 1;
        objects.push_back({
            {"id", id.str()}, {"name", "StageCollider_" + std::to_string(i + 1)}, {"tag", "Untagged"},
            {"renderLayer", "Default"}, {"collisionLayer", "Field"},
            {"transform", {{"position", Vec3(box.center)}, {"rotationDegrees", Vec3(box.rotationDegrees)}, {"scale", {1.0f,1.0f,1.0f}}}},
            {"collider", {{"type", "Box"}, {"center", {0.0f,0.0f,0.0f}}, {"boxSize", Vec3(box.size)}, {"sphereRadius", 0.5f}}},
            {"modelPath", ""}, {"materialName", ""}
        });
    }

    const json root = {{"type", "Level"}, {"formatVersion", 1}, {"name", output.stem().stem().string()}, {"data", {{"objects", objects}}}};
    std::ofstream stream(output);
    if (!stream) return 2;
    stream << root.dump(4) << '\n';
    std::cout << "Generated " << boxes.size() << " box colliders in " << output.string() << '\n';
}
