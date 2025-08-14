#include "TileComponent.h"
#include "../components/SceneManager.h"


void TileComponent::update() 
{
	
}

void TileComponent::SetupNewMaterial(std::string _PathTexture) {
	GameObject* newObject = new GameObject("Cube.fbx");
	SceneManager::getInstance().getActiveScene()->addGameObject(newObject);
}