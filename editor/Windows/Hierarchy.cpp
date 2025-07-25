#include "Hierarchy.h"
#include "components/SceneManager.h"
#include "components/GameObject.h"
#include "Selection.h"

void Hierarchy::OnRenderGUI() {
	ImGui::Begin("Hierarchy", &isOpen);
	
	for (size_t i = 0; i < SceneManager::getInstance().getActiveScene()->getGameObjects().size(); i++)
	{
		ImGui::PushID(SceneManager::getInstance().getActiveScene()->getGameObjects()[i]->ObjectID.c_str());
		if (ImGui::Selectable(SceneManager::getInstance().getActiveScene()->getGameObjects()[i]->Name.c_str())) {
			
			if (SceneManager::getInstance().getActiveScene()->getGameObjects()[i] == Selection::GameObjectSelect) {
				Selection::GameObjectSelect = nullptr;
			}
			else {
				Selection::GameObjectSelect = SceneManager::getInstance().getActiveScene()->getGameObjects()[i];
			}
		}
		ImGui::PopID();
	}

	ImGui::End();
}