#include "Hierarchy.h"
#include "components/SceneManager.h"
#include "components/GameObject.h"
#include "Selection.h"

void Hierarchy::OnRenderGUI() {
	ImGui::Begin("Hierarchy", &isOpen);
	
	// Obtener la escena activa
	auto* activeScene = SceneManager::getInstance().getActiveScene();
	if (!activeScene) {
		ImGui::Text("No active scene");
		ImGui::End();
		return;
	}
	
	const auto& gameObjects = activeScene->getGameObjects();
	
	// Mostrar información de selección actual
	if (Selection::GameObjectSelect) {
		ImGui::TextColored(ImVec4(0.2f, 0.6f, 1.0f, 1.0f), "Selected: %s", Selection::GameObjectSelect->Name.c_str());
	} else {
		ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No object selected");
	}
	ImGui::Separator();
	
	// Mostrar mensaje si no hay objetos
	if (gameObjects.empty()) {
		ImGui::Text("No GameObjects in scene");
		ImGui::End();
		return;
	}
	
	for (size_t i = 0; i < gameObjects.size(); i++)
	{
		GameObject* gameObject = gameObjects[i];
		ImGui::PushID(gameObject->ObjectID.c_str());
		
		// Verificar si este objeto está seleccionado
		bool isSelected = (gameObject == Selection::GameObjectSelect);
		
		// Aplicar color diferente para objetos seleccionados
		if (isSelected) {
			ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.6f, 1.0f, 0.8f));
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.3f, 0.7f, 1.0f, 0.9f));
			ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.1f, 0.5f, 0.9f, 1.0f));
		}
		
		// Crear texto con icono para objetos seleccionados
		std::string displayText = gameObject->Name;
		if (isSelected) {
			displayText = "► " + displayText;
		}
		
		if (ImGui::Selectable(displayText.c_str(), isSelected)) {
			if (gameObject == Selection::GameObjectSelect) {
				Selection::GameObjectSelect = nullptr;
			}
			else {
				Selection::GameObjectSelect = gameObject;
			}
		}
		
		// Menú contextual con clic derecho
		if (ImGui::BeginPopupContextItem()) {
			if (ImGui::MenuItem("Select")) {
				Selection::GameObjectSelect = gameObject;
			}
			if (ImGui::MenuItem("Deselect")) {
				Selection::GameObjectSelect = nullptr;
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Delete")) {
				// TODO: Implementar eliminación de GameObject
				// Por ahora solo deseleccionar
				if (Selection::GameObjectSelect == gameObject) {
					Selection::GameObjectSelect = nullptr;
				}
			}
			ImGui::EndPopup();
		}
		
		// Restaurar colores si estaba seleccionado
		if (isSelected) {
			ImGui::PopStyleColor(3);
		}
		
		ImGui::PopID();
	}

	ImGui::End();
}