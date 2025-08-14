#include "Gizmos.h"
#include "components/SceneManager.h"
#include "components/GameObject.h"
#include "render/Camera.h"
#include "render/Framebuffer.h"
#include "Selection.h"
#include "../EUI/EditorInfo.h"
#include "../SceneSaver.h"


void Gizmos::OnRenderGUI() {
    ImGui::Begin("Gizmos");
    
    // Botón de Play/Stop
    if (EditorInfo::IsPlaying) {
        if (ImGui::Button("Stop")) {
            std::cout << "Gizmos: Stop button pressed - Reloading scene..." << std::endl;
            
            // Verificar que tenemos una ruta de escena válida
            if (EditorInfo::currentScenePath.empty()) {
                std::cerr << "Gizmos: ERROR - No current scene path available for reload!" << std::endl;
                std::cerr << "Gizmos: Please save the scene first or load a scene file" << std::endl;
                return;
            }
            
            std::cout << "Gizmos: Current scene path: " << EditorInfo::currentScenePath << std::endl;
            
            // Detener el modo de juego PRIMERO
            EditorInfo::IsPlaying = false;
            std::cout << "Gizmos: Game mode stopped" << std::endl;
            
            // Recargar la escena usando SceneSaver (que ya incluye limpieza automática)
            std::cout << "Gizmos: Starting scene reload..." << std::endl;
            bool success = SceneSaver::LoadScene(EditorInfo::currentScenePath);
            
            if (success) {
                std::cout << "Gizmos: Scene reloaded successfully!" << std::endl;
                std::cout << "Gizmos: Scene is now ready for editing" << std::endl;
            } else {
                std::cerr << "Gizmos: ERROR - Failed to reload scene!" << std::endl;
                std::cerr << "Gizmos: The scene may be in an inconsistent state" << std::endl;
            }
        }
    }
    else {
        if (ImGui::Button("Play")) {
            std::cout << "Gizmos: Play button pressed - Starting game mode" << std::endl;
            EditorInfo::IsPlaying = true;
        }
    }
    
    ImGui::Separator();
    
    // Botón de recarga manual (útil para debugging)
    if (ImGui::Button("Reload Scene")) {
        std::cout << "Gizmos: Manual reload button pressed..." << std::endl;
        
        if (EditorInfo::currentScenePath.empty()) {
            std::cerr << "Gizmos: ERROR - No current scene path available for reload!" << std::endl;
            std::cerr << "Gizmos: Please save the scene first or load a scene file" << std::endl;
            return;
        }
        
        std::cout << "Gizmos: Manually reloading scene from: " << EditorInfo::currentScenePath << std::endl;
        bool success = SceneSaver::LoadScene(EditorInfo::currentScenePath);
        
        if (success) {
            std::cout << "Gizmos: Manual scene reload successful!" << std::endl;
        } else {
            std::cerr << "Gizmos: ERROR - Manual scene reload failed!" << std::endl;
        }
    }
    
    // Mostrar información de la escena actual
    ImGui::Separator();
    ImGui::Text("Current Scene: %s", EditorInfo::currentScenePath.empty() ? "None" : EditorInfo::currentScenePath.c_str());
    ImGui::Text("Game Mode: %s", EditorInfo::IsPlaying ? "Playing" : "Editing");
    
    ImGui::End();
}