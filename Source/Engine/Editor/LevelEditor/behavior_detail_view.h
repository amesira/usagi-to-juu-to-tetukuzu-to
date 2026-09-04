// behavior_detail_view.h
// ・BehaviorComponentの詳細表示用のヘルパー関数を定義するヘッダ
#pragma once
#include "Engine/Core/component.h"
#include "Engine/Editor/i_editor_window.h"

namespace BehaviorDetailView {
    inline bool BeginSection(Component* comp, const char* name, bool useEnableSetting)
    {
        ImGui::PushID(comp);
        ImGui::Separator();

        bool open = ImGui::CollapsingHeader(name, ImGuiTreeNodeFlags_DefaultOpen);
        bool enable = true;
        if (open && useEnableSetting) {
            enable = comp->GetEnable();
            if (ImGui::Checkbox("Enable", &enable)) {
                comp->SetEnable(enable);
            }
        }

        if (enable) {
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1.0f);
        }
        else {
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.4f);
        }

        return open;
    }

    inline void EndSection()
    {
        ImGui::PopStyleVar();
        ImGui::PopID();
    }
}