#pragma once 
#include <iostream>
#include <memory>
#include <vector>
#include <unordered_map>
#include "../core/CoreExporter.h"
#include "../render/Texture.h"
#include "../render/Material.h"
#include "Component.h"
#include "../components/GameObject.h"

struct MANTRAXCORE_API SpriteArray {
    std::string state_name = "New State";
    std::vector<std::string> texturePaths; // Rutas de las texturas en lugar de objetos Texture
};

class MANTRAXCORE_API SpriteAnimator : public Component {
public:
	std::string animator_file = "";
	std::vector<SpriteArray> SpriteStates = std::vector<SpriteArray>();
	std::string currentState = "None";
	std::string getComponentName() const override {
		return "Sprite Animator";
	}

	// Animation playback properties
	bool isPlaying = false;
	float animationSpeed = 1.0f;
	int currentFrame = 0;
	float frameTimer = 0.0f;
	std::string playbackState = "None"; // State to play during animation
	
	// Material propio del SpriteAnimator
	std::shared_ptr<Material> spriteMaterial;
	
	// Debug flags
	bool debugMode = false;
	bool forceMaterialUpdate = false;
	
	// Array de texturas persistentes
	std::unordered_map<std::string, std::shared_ptr<Texture>> persistentTextures;
	
	// Constructor y destructor
	SpriteAnimator();
	~SpriteAnimator();
	
	// Métodos para gestionar el material
	void createMaterial(const std::string& materialName = "SpriteMaterial");
	void setMaterial(std::shared_ptr<Material> material);
	std::shared_ptr<Material> getMaterial() const { return spriteMaterial; }
	
	// Métodos para configurar el material
	void setSpriteAlbedo(const glm::vec3& color);
	void setSpriteMetallic(float value);
	void setSpriteRoughness(float value);
	void setSpriteEmissive(const glm::vec3& color);
	void setSpriteTiling(const glm::vec2& tiling);
	
	// Métodos para gestionar texturas persistentes
	void loadTexture(const std::string& texturePath);
	std::shared_ptr<Texture> getTexture(const std::string& texturePath);
	void preloadAllTextures();
	void clearTextureCache();
	
	// Método para actualizar la textura del material con el frame actual
	void updateMaterialTexture();
	
	// Animation playback methods
	void playAnimation(const std::string& stateName = "");
	void stopAnimation();
	void pauseAnimation();
	void setAnimationSpeed(float speed);
	void setCurrentFrame(int frame);
	int getCurrentFrame() const { return currentFrame; }
	bool getIsPlaying() const { return isPlaying; }
	std::string getPlaybackState() const { return playbackState; }
	
	// State management methods
	void addSpriteState(const std::string& stateName);
	void setCurrentState(const std::string& stateName);
	void addTextureToState(const std::string& stateName, const std::string& texturePath);

	// Métodos de debugging
	void enableDebugMode(bool enable = true) { debugMode = enable; }
	bool isDebugModeEnabled() const { return debugMode; }
	void forceUpdate() { forceMaterialUpdate = true; }
	
	// Métodos de validación
	bool isValidState(const std::string& stateName) const;
	bool hasValidTextures(const std::string& stateName) const;
	bool isMaterialValid() const;
	
	// Métodos de información
	std::string getDebugInfo() const;
	void printDebugInfo() const;

	// Métodos de serialización para el inspector
	std::string serializeComponent() const override;
	void deserialize(const std::string& data) override;

	// Métodos para cargar configuraciones desde archivos .animator
	bool loadFromAnimatorFile(const std::string& filePath);
	bool loadFromAnimatorData(const nlohmann::json& animatorData);
	std::string getAnimatorFilePath() const;
	
	// Métodos para guardar y cargar el componente completo
	bool saveToFile(const std::string& filePath) const;
	bool loadFromFile(const std::string& filePath);

	void defines() override;
	void start() override;
	void update() override;
};