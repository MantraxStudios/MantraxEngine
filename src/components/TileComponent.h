#pragma once 
#include <iostream>
#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include "Component.h"
#include "../core/CoreExporter.h"
#include "../render/Texture.h"
#include "../render/Material.h"
#include "../components/GameObject.h"

class MANTRAXCORE_API TileComponent : public Component {
public:
	std::string getComponentName() const override {
		return "Tile Component";
	}

	void update() override;
	void SetTile(Material _Mat);
	void SetupNewMaterial(std::string _PathTexture);
};