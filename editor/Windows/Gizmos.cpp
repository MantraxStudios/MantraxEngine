#include "Gizmos.h"
#include "components/SceneManager.h"
#include "components/GameObject.h"
#include "render/Camera.h"
#include "render/Framebuffer.h"
#include "Selection.h"
#include "../EUI/EditorInfo.h"

void Gizmos::OnRenderGUI() {
    ImGui::Begin("Gizmos");

    if (Selection::GameObjectSelect != nullptr) {
        ImGui::Text("Selected: %s", Selection::GameObjectSelect->Name.c_str());
        
        ImGui::Text("Gizmo Controls:");
        ImGui::Text("T - Translate");
        ImGui::Text("R - Rotate");
        ImGui::Text("S - Scale");
        ImGui::Text("L - Toggle Local/World");
    } else {
        ImGui::Text("No object selected");
    }

    ImGui::End();
}