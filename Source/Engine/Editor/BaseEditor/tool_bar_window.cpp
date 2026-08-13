//===================================================
// tool_bar_window.cpp
// 
// Author：Miu Kitamura
// Date  ：2026/03/12
//===================================================
#include "tool_bar_window.h"
#include "Engine/engine.h"

#include "Engine/Editor/editor_context.h"
#include "Engine/Editor/editor_window_manager.h"
#include "Engine/render_view.h"
#include "Engine/Core/game_object.h"

#include "Engine/Device/mi_fps.h"

void ToolBarWindow::Draw()
{
    const char* collapseLabel = m_editorContext->toolbarExpanded ? "^" : "v";
    if (ImGui::Button(collapseLabel)) {
        m_editorContext->toolbarExpanded = !m_editorContext->toolbarExpanded;
    }

    if (!m_editorContext->toolbarExpanded) {
        return;
    }

    ImGui::SameLine();
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 6));

    ImGui::Text("Mi Engine v2 : ");
    ImGui::SameLine();

    ImGui::Text("FPS: %.1f", FPS_GetFPS());
    ImGui::SameLine();

    const char* items[] = { "Edit", "Play" };
    int currentIndex = static_cast<int>(m_editorContext->currentEditorMode);

    ImGui::SetNextItemWidth(120);
    if (ImGui::Combo("##Mode", &currentIndex, items, IM_ARRAYSIZE(items))) {
        // currentIndex が変わったとき
        EditorContext::EditorMode newMode = static_cast<EditorContext::EditorMode>(currentIndex);
        m_editorContext->currentEditorMode = newMode;
    }
    ImGui::SameLine();

    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    ImGui::SameLine();

    ImGui::TextUnformatted("Main View:");
    ImGui::SameLine();

    if (ImGui::RadioButton(
        "Game",
        m_editorContext->mainViewMode == EditorContext::MainViewMode::Game)) {
        m_editorContext->mainViewMode = EditorContext::MainViewMode::Game;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton(
        "Scene",
        m_editorContext->mainViewMode == EditorContext::MainViewMode::Scene)) {
        m_editorContext->mainViewMode = EditorContext::MainViewMode::Scene;
    }
    ImGui::SameLine();

    if (ImGui::Button("Windows")) {
        ImGui::OpenPopup("EditorWindowMenu");
    }

    if (ImGui::BeginPopup("EditorWindowMenu")) {
        if (m_windowManager) {
            auto drawWindowToggle = [this](const char* label, EditorWindowId id) {
                bool open = m_windowManager->IsOpen(id);
                if (ImGui::MenuItem(label, nullptr, open)) {
                    m_windowManager->Toggle(id);
                }
            };

            drawWindowToggle("Hierarchy", EditorWindowId::Hierarchy);
            drawWindowToggle("Runtime Inspector", EditorWindowId::Inspector);
            drawWindowToggle("Debug", EditorWindowId::Debug);
            ImGui::Separator();
            drawWindowToggle("Game View", EditorWindowId::GameView);
            drawWindowToggle("Scene View", EditorWindowId::SceneView);
            drawWindowToggle("Canvas View", EditorWindowId::CanvasView);
            drawWindowToggle("Settings", EditorWindowId::Settings);
            ImGui::Separator();
            drawWindowToggle("Particle System Editor", EditorWindowId::ParticleSystemEditor);
            drawWindowToggle("Data Editor", EditorWindowId::DataEditor);
        }
        ImGui::EndPopup();
    }

    ImGui::SameLine();
    if (ImGui::Button("Reload Scene")) {
        m_editorContext->triggerSceneReload = true;
    }

    ImGui::PopStyleVar();
}
