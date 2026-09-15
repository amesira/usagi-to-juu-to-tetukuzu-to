//---------------------------------------------------
// File  ：_/Editor/LevelEditor/level_editor_window.h
// Date  ：2026/09/02
// Author：Miu Kitamura
// 
// ・レベルエディタ
//---------------------------------------------------
#pragma once
#include "Engine/Editor/i_editor_window.h"
#include "level_editor_document.h"

#include <array>
#include <string>

class GameObject;
class IScene;

class LevelEditorWindow : public IEditorWindow {
private:
    LevelEditorDocument m_document;

    std::array<char, 512> m_pathBuffer{};
    std::string m_selectedLevelObjectId;
    unsigned int m_selectedRuntimeObjectId = static_cast<unsigned int>(-1);
    
    IScene* m_appliedScene = nullptr;

    bool m_autoSave = true;
    int m_autoSaveFrameCounter = 0;
    float m_levelScaleFactor = 1.0f;
    bool m_levelScalePreviewActive = false;

public:
    explicit LevelEditorWindow(EditorContext* editorContext);
    void Draw() override;
    void OnWindowClosed() override {}
    void OnSceneDestroyed();

private:
    void DrawToolbar();
    void DrawLevelScaleControls();
    void DrawLevelObjectsTab();
    void DrawRuntimeObjectsTab();
    void DrawLevelHierarchy();
    void DrawLevelInspector();
    void DrawRuntimeHierarchy();
    void DrawRuntimeInspector();

    GameObject* FindLevelObject(const std::string& id) const;
    GameObject* FindRuntimeObject() const;
    bool ApplyObjectData(GameObject* object, const LevelObjectData& data);
    void PreviewLevelScale(float factor);
    void CancelLevelScalePreview();
    void ApplyAllObjectData();
    void RebuildLevelObjects();
    void DestroyLevelObjects();
    void SelectLevelObject(const std::string& id);
    void SyncPathBuffer();
};
