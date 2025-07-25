#include "Hierarchy.h"
#include "components/SceneManager.h"
#include "components/GameObject.h"

void Hierarchy::OnRenderGUI() {
	ImGui::Begin("Hierarchy", &isOpen);
	
	for (size_t i = 0; i < SceneManager::getInstance().getActiveScene()->getGameObjects().size(); i++)
	{
		ImGui::PushID(SceneManager::getInstance().getActiveScene()->getGameObjects()[i]->ObjectID.c_str());
		ImGui::Selectable(SceneManager::getInstance().getActiveScene()->getGameObjects()[i]->Name.c_str());
		ImGui::PopID();
	}

	ImGui::End();
}