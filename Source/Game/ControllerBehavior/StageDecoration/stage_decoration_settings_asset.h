#pragma once
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>
#include "Engine/Asset/DataAsset/data_asset.h"
#include "Engine/Asset/DataAsset/data_asset_type_id.h"
#include "Engine/Asset/Schema/field_master.h"
#include "Engine/Editor/Schema/field_editor.h"
#include "Engine/Editor/Schema/enum_field_editor.h"

namespace StageDecorationSettings {
    enum class Shader { Lit, Unlit };
    struct ObjectSettings {
        std::string name = "Decoration";
        std::string modelPath;
        bool enabled = true;
        DirectX::XMFLOAT3 position = {};
        DirectX::XMFLOAT3 rotationDegrees = {};
        DirectX::XMFLOAT3 scale = {1, 1, 1};
        Shader shader = Shader::Lit;
        bool rotationEnabled = false;
        DirectX::XMFLOAT3 rotationAxis = {0, 1, 0}; // 初期姿勢に対するローカル軸
        float rotationSpeedDegrees = 30;
    };
    struct Data {
        bool useUnscaledTime = false;
        std::vector<ObjectSettings> objects;
    };
    inline const auto& GetObjectSchema() {
        static const auto schema = FieldSchema{
            MakeField("name", "Name", &ObjectSettings::name),
            MakeField("modelPath", "Model Path", &ObjectSettings::modelPath),
            MakeField("enabled", "Enabled", &ObjectSettings::enabled),
            MakeField("position", "Position", &ObjectSettings::position, DragFieldOptions{.dragSpeed = 0.1f, .minValue = -10000, .maxValue = 10000}),
            MakeField("rotationDegrees", "Initial Rotation (degrees)", &ObjectSettings::rotationDegrees, DragFieldOptions{.dragSpeed = 1, .minValue = -360, .maxValue = 360}),
            MakeField("scale", "Scale", &ObjectSettings::scale, DragFieldOptions{.dragSpeed = 0.01f, .minValue = 0.001f, .maxValue = 1000}),
            MakeField("shader", "Shader", &ObjectSettings::shader, EnumFieldOptions<Shader>{{{Shader::Lit, "Lit", "Lit"}, {Shader::Unlit, "Unlit", "Unlit"}}}),
            MakeField("rotationEnabled", "Continuous Rotation", &ObjectSettings::rotationEnabled),
            MakeField("rotationAxis", "Local Rotation Axis", &ObjectSettings::rotationAxis, DragFieldOptions{.dragSpeed = 0.01f, .minValue = -1, .maxValue = 1}),
            MakeField("rotationSpeedDegrees", "Rotation Speed (degrees/s)", &ObjectSettings::rotationSpeedDegrees, DragFieldOptions{.dragSpeed = 1, .minValue = -3600, .maxValue = 3600})
        };
        return schema;
    }
    inline void Sanitize(ObjectSettings& object) {
        auto finite = [](float value, float fallback) { return std::isfinite(value) ? value : fallback; };
        object.position = {finite(object.position.x, 0), finite(object.position.y, 0), finite(object.position.z, 0)};
        object.rotationDegrees = {std::remainder(finite(object.rotationDegrees.x, 0), 360.0f), std::remainder(finite(object.rotationDegrees.y, 0), 360.0f), std::remainder(finite(object.rotationDegrees.z, 0), 360.0f)};
        object.scale = {(std::max)(0.001f, finite(object.scale.x, 1)), (std::max)(0.001f, finite(object.scale.y, 1)), (std::max)(0.001f, finite(object.scale.z, 1))};
        object.rotationAxis = {finite(object.rotationAxis.x, 0), finite(object.rotationAxis.y, 0), finite(object.rotationAxis.z, 0)};
        object.rotationSpeedDegrees = std::clamp(finite(object.rotationSpeedDegrees, 0), -3600.0f, 3600.0f);
    }
    inline nlohmann::json Serialize(const Data& data) {
        auto json = nlohmann::json{{"useUnscaledTime", data.useUnscaledTime}, {"objects", nlohmann::json::array()}};
        for (const auto& object : data.objects) json["objects"].push_back(FieldSerialization::SerializeFields(object, GetObjectSchema()));
        return json;
    }
    inline bool Deserialize(const nlohmann::json& json, Data& destination) {
        if (!json.is_object()) return false;
        auto data = destination;
        if (json.contains("useUnscaledTime")) {
            if (!json["useUnscaledTime"].is_boolean()) return false;
            data.useUnscaledTime = json["useUnscaledTime"].get<bool>();
        }
        if (json.contains("objects")) {
            const auto& objects = json["objects"];
            if (!objects.is_array() || objects.size() > 1024) return false;
            data.objects.clear();
            for (const auto& jsonObject : objects) {
                if (!jsonObject.is_object()) return false;
                ObjectSettings object;
                if (!FieldSerialization::DeserializeFields(jsonObject, object, GetObjectSchema())) return false;
                Sanitize(object);
                data.objects.push_back(std::move(object));
            }
        }
        destination = std::move(data);
        return true;
    }
    inline DirectX::XMFLOAT4 ResolveRotation(const DirectX::XMFLOAT4& initial, const DirectX::XMFLOAT3& axis, float angleDegrees) {
        using namespace DirectX;
        const auto vector = XMLoadFloat3(&axis);
        if (XMVectorGetX(XMVector3LengthSq(vector)) < 0.000001f) return initial;
        const auto local = XMMatrixRotationAxis(vector, XMConvertToRadians(angleDegrees));
        XMFLOAT4 result;
        XMStoreFloat4(&result, XMQuaternionNormalize(XMQuaternionRotationMatrix(local * XMMatrixRotationQuaternion(XMLoadFloat4(&initial)))));
        return result;
    }
}

class StageDecorationSettingsAsset : public DataAsset {
    StageDecorationSettings::Data m_data;
public:
    StageDecorationSettingsAsset() : DataAsset(DataAssetTypeID::getTypeID<StageDecorationSettingsAsset>(), "StageDecorationSettingsAsset", 0) {}
    const StageDecorationSettings::Data& GetData() const { return m_data; }
    std::unique_ptr<DataAsset> CreateDefaultInstance() const override { return std::make_unique<StageDecorationSettingsAsset>(); }
    nlohmann::json SerializeData() const override { return StageDecorationSettings::Serialize(m_data); }
    bool DeserializeDataToApply(const nlohmann::json& json) override { return StageDecorationSettings::Deserialize(json, m_data); }
    bool DrawDataOnEditor() override {
        bool changed = ImGui::Checkbox("Use Unscaled Time", &m_data.useUnscaledTime);
        for (size_t i = 0; i < m_data.objects.size(); ++i) {
            ImGui::PushID(static_cast<int>(i));
            const auto title = std::to_string(i) + ": " + m_data.objects[i].name;
            if (ImGui::TreeNode(title.c_str())) {
                if (FieldEditor::DrawFields(m_data.objects[i], StageDecorationSettings::GetObjectSchema())) {
                    StageDecorationSettings::Sanitize(m_data.objects[i]); changed = true;
                }
                if (ImGui::Button("Remove Object")) {
                    m_data.objects.erase(m_data.objects.begin() + i);
                    changed = true;
                    ImGui::TreePop(); ImGui::PopID(); break;
                }
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
        if (m_data.objects.size() < 1024 && ImGui::Button("Add Object")) { m_data.objects.emplace_back(); changed = true; }
        return changed;
    }
};
