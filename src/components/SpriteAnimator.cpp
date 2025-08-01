#include "SpriteAnimator.h"
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Constructor
SpriteAnimator::SpriteAnimator() {
    // Crear material por defecto
    createMaterial();
}

// Destructor
SpriteAnimator::~SpriteAnimator() {
    // El shared_ptr se libera automáticamente
}

// Crear material propio
void SpriteAnimator::createMaterial(const std::string& materialName) {
    spriteMaterial = std::make_shared<Material>(materialName);
    
    // Configuración por defecto para sprites
    spriteMaterial->setAlbedo(glm::vec3(1.0f, 1.0f, 1.0f)); // Color blanco
    spriteMaterial->setMetallic(0.0f); // No metálico
    spriteMaterial->setRoughness(0.5f); // Semi-rugoso
    spriteMaterial->setEmissive(glm::vec3(0.0f, 0.0f, 0.0f)); // Sin emisión
    spriteMaterial->setTiling(glm::vec2(1.0f, 1.0f)); // Sin tiling
}

// Establecer material externo
void SpriteAnimator::setMaterial(std::shared_ptr<Material> material) {
    spriteMaterial = material;
}

// Configurar propiedades del material
void SpriteAnimator::setSpriteAlbedo(const glm::vec3& color) {
    if (spriteMaterial) {
        spriteMaterial->setAlbedo(color);
    }
}

void SpriteAnimator::setSpriteMetallic(float value) {
    if (spriteMaterial) {
        spriteMaterial->setMetallic(value);
    }
}

void SpriteAnimator::setSpriteRoughness(float value) {
    if (spriteMaterial) {
        spriteMaterial->setRoughness(value);
    }
}

void SpriteAnimator::setSpriteEmissive(const glm::vec3& color) {
    if (spriteMaterial) {
        spriteMaterial->setEmissive(color);
    }
}

void SpriteAnimator::setSpriteTiling(const glm::vec2& tiling) {
    if (spriteMaterial) {
        spriteMaterial->setTiling(tiling);
    }
}

// Método para actualizar la textura del material con el frame actual
void SpriteAnimator::updateMaterialTexture() {
    if (!spriteMaterial) {
        std::cout << "Warning: No material available for SpriteAnimator" << std::endl;
        return;
    }
    
    // Buscar el estado actual
    bool stateFound = false;
    for (const auto& state : SpriteStates) {
        if (state.state_name == currentState && !state.texturePaths.empty()) {
            // Verificar que el frame actual sea válido
            if (currentFrame < static_cast<int>(state.texturePaths.size())) {
                const std::string& texturePath = state.texturePaths[currentFrame];
                
                // Obtener la textura del cache persistente
                auto currentTexture = getTexture(texturePath);
                
                // Verificar que la textura sea válida
                if (currentTexture && currentTexture->getID() != 0) {
                    spriteMaterial->setAlbedoTexture(currentTexture);
                    if (debugMode) {
                        std::cout << "Actualizando textura del material para estado: " << currentState 
                                  << " frame: " << currentFrame << " path: " << texturePath << std::endl;
                    }
                    stateFound = true;
                } else {
                    if (debugMode) {
                        std::cout << "Warning: Invalid texture for state: " << currentState 
                                  << " frame: " << currentFrame << " path: " << texturePath << std::endl;
                    }
                }
            } else {
                if (debugMode) {
                    std::cout << "Warning: Invalid frame index for state: " 
                              << currentState << " frame: " << currentFrame << std::endl;
                }
            }
            break;
        }
    }
    
    if (!stateFound && debugMode) {
        std::cout << "Warning: State '" << currentState << "' not found or has no textures" << std::endl;
    }
}

// Animation playback methods
void SpriteAnimator::playAnimation(const std::string& stateName) {
    if (!stateName.empty()) {
        playbackState = stateName;
    } else {
        playbackState = currentState;
    }
    
    // Verify the state exists
    bool stateExists = false;
    for (const auto& state : SpriteStates) {
        if (state.state_name == playbackState) {
            stateExists = true;
            break;
        }
    }
    
    if (!stateExists) {
        std::cout << "Warning: State '" << playbackState << "' not found for animation playback" << std::endl;
        return;
    }
    
    isPlaying = true;
    currentFrame = 0;
    frameTimer = 0.0f;
    std::cout << "Starting animation playback for state: " << playbackState << std::endl;
}

void SpriteAnimator::stopAnimation() {
    isPlaying = false;
    currentFrame = 0;
    frameTimer = 0.0f;
    std::cout << "Animation playback stopped" << std::endl;
}

void SpriteAnimator::pauseAnimation() {
    isPlaying = false;
    std::cout << "Animation playback paused" << std::endl;
}

void SpriteAnimator::setAnimationSpeed(float speed) {
    animationSpeed = speed;
    if (animationSpeed < 0.1f) animationSpeed = 0.1f;
    if (animationSpeed > 5.0f) animationSpeed = 5.0f;
}

void SpriteAnimator::setCurrentFrame(int frame) {
    currentFrame = frame;
    if (currentFrame < 0) currentFrame = 0;
    
    // Find the current playback state and get its texture count
    for (const auto& state : SpriteStates) {
        if (state.state_name == playbackState) {
            if (currentFrame >= static_cast<int>(state.texturePaths.size())) {
                currentFrame = static_cast<int>(state.texturePaths.size()) - 1;
            }
            break;
        }
    }
}

// State management methods
void SpriteAnimator::addSpriteState(const std::string& stateName) {
    // Verificar si el estado ya existe
    for (const auto& state : SpriteStates) {
        if (state.state_name == stateName) {
            if (debugMode) {
                std::cout << "State '" << stateName << "' already exists" << std::endl;
            }
            return;
        }
    }
    
    // Crear nuevo estado
    SpriteArray newState;
    newState.state_name = stateName;
    SpriteStates.push_back(newState);
    
    if (debugMode) {
        std::cout << "Added new state: " << stateName << std::endl;
    }
}

void SpriteAnimator::setCurrentState(const std::string& stateName) {
    // Verificar si el estado existe
    bool stateExists = false;
    for (const auto& state : SpriteStates) {
        if (state.state_name == stateName) {
            stateExists = true;
            break;
        }
    }
    
    if (!stateExists) {
        if (debugMode) {
            std::cout << "Warning: State '" << stateName << "' not found" << std::endl;
        }
        return;
    }
    
    currentState = stateName;
    if (debugMode) {
        std::cout << "Current state set to: " << stateName << std::endl;
    }
}

void SpriteAnimator::addTextureToState(const std::string& stateName, const std::string& texturePath) {
    // Buscar el estado
    for (auto& state : SpriteStates) {
        if (state.state_name == stateName) {
            state.texturePaths.push_back(texturePath);
            if (debugMode) {
                std::cout << "Added texture '" << texturePath << "' to state '" << stateName << "'" << std::endl;
            }
            return;
        }
    }
    
    if (debugMode) {
        std::cout << "Warning: State '" << stateName << "' not found, cannot add texture" << std::endl;
    }
}

void SpriteAnimator::start() {
    // Asegurar que tenemos un material
    if (!spriteMaterial) {
        createMaterial();
    }
    
    // Precargar todas las texturas
    preloadAllTextures();
    
    // Configuración inicial del material
    updateMaterialTexture();
}

// Métodos de debugging y validación
bool SpriteAnimator::isValidState(const std::string& stateName) const {
    for (const auto& state : SpriteStates) {
        if (state.state_name == stateName) {
            return true;
        }
    }
    return false;
}

bool SpriteAnimator::hasValidTextures(const std::string& stateName) const {
    for (const auto& state : SpriteStates) {
        if (state.state_name == stateName) {
            return !state.texturePaths.empty();
        }
    }
    return false;
}

bool SpriteAnimator::isMaterialValid() const {
    return spriteMaterial != nullptr && spriteMaterial->isValid();
}

std::string SpriteAnimator::getDebugInfo() const {
    std::string info = "SpriteAnimator Debug Info:\n";
    info += "- Current State: " + currentState + "\n";
    info += "- Playback State: " + playbackState + "\n";
    info += "- Is Playing: " + std::to_string(isPlaying) + "\n";
    info += "- Current Frame: " + std::to_string(currentFrame) + "\n";
    info += "- Animation Speed: " + std::to_string(animationSpeed) + "\n";
    info += "- Frame Timer: " + std::to_string(frameTimer) + "\n";
    info += "- Material Valid: " + std::to_string(isMaterialValid()) + "\n";
    info += "- States Count: " + std::to_string(SpriteStates.size()) + "\n";
    
    for (size_t i = 0; i < SpriteStates.size(); ++i) {
        info += "- State " + std::to_string(i) + ": " + SpriteStates[i].state_name + 
                " (Textures: " + std::to_string(SpriteStates[i].texturePaths.size()) + ")\n";
    }
    
    if (spriteMaterial) {
        info += "- Material Name: " + spriteMaterial->getName() + "\n";
        info += "- Material Albedo: (" + std::to_string(spriteMaterial->getAlbedo().x) + ", " +
                std::to_string(spriteMaterial->getAlbedo().y) + ", " +
                std::to_string(spriteMaterial->getAlbedo().z) + ")\n";
        info += "- Has Albedo Texture: " + std::to_string(spriteMaterial->hasAlbedoTexture()) + "\n";
    }
    
    return info;
}

void SpriteAnimator::printDebugInfo() const {
    if (debugMode) {
        std::cout << getDebugInfo() << std::endl;
    }
}

// Actualizar el método update para incluir debugging
void SpriteAnimator::update() {
    if (debugMode) {
        printDebugInfo();
    }

    // Verificar que el GameObject tenga el material correcto
    if (getOwner() && getOwner()->getMaterial() != this->getMaterial()) {
        getOwner()->setMaterial(this->getMaterial());
        if (debugMode) {
            std::cout << "Material actualizado en GameObject" << std::endl;
        }
    }

    // Actualizar la textura del material si cambió el estado o se fuerza la actualización
    static std::string lastState = "";
    if (currentState != lastState || forceMaterialUpdate) {
        updateMaterialTexture();
        lastState = currentState;
        forceMaterialUpdate = false;
        if (debugMode) {
            std::cout << "Estado cambiado o actualización forzada: " << currentState << std::endl;
        }
    }

    // Handle animation playback
    if (isPlaying && !playbackState.empty()) {
        // Find the playback state
        for (const auto& state : SpriteStates) {
            if (state.state_name == playbackState && !state.texturePaths.empty()) {
                // Update frame timer
                frameTimer += animationSpeed * 0.016f; // Assuming 60 FPS
                
                // Check if it's time to advance to next frame
                if (frameTimer >= 1.0f) {
                    currentFrame++;
                    frameTimer = 0.0f;
                    
                    // Loop back to first frame if we've reached the end
                    if (currentFrame >= static_cast<int>(state.texturePaths.size())) {
                        currentFrame = 0;
                    }
                    
                    // Update the material texture with the current frame
                    if (currentFrame < static_cast<int>(state.texturePaths.size())) {
                        const std::string& texturePath = state.texturePaths[currentFrame];
                        auto currentTexture = getTexture(texturePath);
                        if (spriteMaterial && currentTexture && currentTexture->getID() != 0) {
                            spriteMaterial->setAlbedoTexture(currentTexture);
                            if (debugMode) {
                                std::cout << "Animation frame updated: " << currentFrame << " for state: " << playbackState << std::endl;
                            }
                        } else {
                            if (debugMode) {
                                std::cout << "Warning: Invalid texture at frame " << currentFrame << " for state: " << playbackState << std::endl;
                            }
                        }
                    }
                }
                break;
            }
        }
    }
}

// Métodos de serialización para el inspector
std::string SpriteAnimator::serializeComponent() const {
    json j;
    
    // Serializar estados de animación
    j["currentState"] = currentState;
    
    // Serializar propiedades de animación
    j["isPlaying"] = isPlaying;
    j["animationSpeed"] = animationSpeed;
    j["currentFrame"] = currentFrame;
    j["frameTimer"] = frameTimer;
    j["playbackState"] = playbackState;
    
    json statesArray = json::array();
    for (const auto& state : SpriteStates) {
        json stateObj;
        stateObj["state_name"] = state.state_name;
        
        // Serializar texturas (solo los paths por ahora)
        json texturesArray = json::array();
        for (const auto& texturePath : state.texturePaths) {
            // Asumiendo que Texture tiene un método getPath() o similar
            // Por ahora serializamos información básica
            json textureObj;
            textureObj["path"] = texturePath; // Placeholder
            texturesArray.push_back(textureObj);
        }
        stateObj["textures"] = texturesArray;
        statesArray.push_back(stateObj);
    }
    j["spriteStates"] = statesArray;
    
    // Serializar propiedades del material si existe
    if (spriteMaterial) {
        json materialObj;
        materialObj["name"] = spriteMaterial->getName();
        materialObj["albedo"] = {spriteMaterial->getAlbedo().x, spriteMaterial->getAlbedo().y, spriteMaterial->getAlbedo().z};
        materialObj["metallic"] = spriteMaterial->getMetallic();
        materialObj["roughness"] = spriteMaterial->getRoughness();
        materialObj["emissive"] = {spriteMaterial->getEmissive().x, spriteMaterial->getEmissive().y, spriteMaterial->getEmissive().z};
        materialObj["tiling"] = {spriteMaterial->getTiling().x, spriteMaterial->getTiling().y};
        j["material"] = materialObj;
    }
    
    // Estado del componente
    j["enabled"] = isEnabled;
    
    try {
        return j.dump();
    }
    catch (const std::exception& e) {
        std::cerr << "SpriteAnimator::serializeComponent error: " << e.what() << std::endl;
        return "{}";
    }
}

void SpriteAnimator::deserialize(const std::string& data) {
    try {
        json j = json::parse(data);
        
        // Deserializar estado actual
        if (j.contains("currentState") && j["currentState"].is_string()) {
            currentState = j["currentState"];
        }
        
        // Deserializar propiedades de animación
        if (j.contains("isPlaying")) {
            isPlaying = j["isPlaying"];
        }
        if (j.contains("animationSpeed")) {
            animationSpeed = j["animationSpeed"];
        }
        if (j.contains("currentFrame")) {
            currentFrame = j["currentFrame"];
        }
        if (j.contains("frameTimer")) {
            frameTimer = j["frameTimer"];
        }
        if (j.contains("playbackState") && j["playbackState"].is_string()) {
            playbackState = j["playbackState"];
        }
        
        // Deserializar estados de animación
        if (j.contains("spriteStates") && j["spriteStates"].is_array()) {
            SpriteStates.clear();
            for (const auto& stateJson : j["spriteStates"]) {
                SpriteArray state;
                if (stateJson.contains("state_name") && stateJson["state_name"].is_string()) {
                    state.state_name = stateJson["state_name"];
                }
                
                // Deserializar texturas (implementación básica)
                if (stateJson.contains("textures") && stateJson["textures"].is_array()) {
                    // Por ahora solo creamos texturas vacías
                    // En una implementación completa, cargarías las texturas desde los paths
                    for (const auto& textureJson : stateJson["textures"]) {
                        if (textureJson.contains("path") && textureJson["path"].is_string()) {
                            state.texturePaths.push_back(textureJson["path"]);
                        }
                    }
                }
                
                SpriteStates.push_back(state);
            }
        }
        
        // Deserializar material
        if (j.contains("material") && j["material"].is_object()) {
            auto materialJson = j["material"];
            
            // Crear o actualizar el material
            if (!spriteMaterial) {
                createMaterial(materialJson.value("name", "SpriteMaterial"));
            }
            
            // Configurar propiedades del material
            if (materialJson.contains("albedo") && materialJson["albedo"].is_array()) {
                auto albedo = materialJson["albedo"];
                if (albedo.size() >= 3) {
                    setSpriteAlbedo(glm::vec3(albedo[0], albedo[1], albedo[2]));
                }
            }
            
            if (materialJson.contains("metallic")) {
                setSpriteMetallic(materialJson["metallic"]);
            }
            
            if (materialJson.contains("roughness")) {
                setSpriteRoughness(materialJson["roughness"]);
            }
            
            if (materialJson.contains("emissive") && materialJson["emissive"].is_array()) {
                auto emissive = materialJson["emissive"];
                if (emissive.size() >= 3) {
                    setSpriteEmissive(glm::vec3(emissive[0], emissive[1], emissive[2]));
                }
            }
            
            if (materialJson.contains("tiling") && materialJson["tiling"].is_array()) {
                auto tiling = materialJson["tiling"];
                if (tiling.size() >= 2) {
                    setSpriteTiling(glm::vec2(tiling[0], tiling[1]));
                }
            }
        }
        
        // Estado del componente
        if (j.contains("enabled")) {
            isEnabled = j.value("enabled", true);
        }
        
        // Actualizar la textura del material después de deserializar
        updateMaterialTexture();
        
    }
    catch (const json::exception& e) {
        std::cerr << "SpriteAnimator::deserialize error: " << e.what() << std::endl;
    }
}

// Métodos para gestionar texturas persistentes
void SpriteAnimator::loadTexture(const std::string& texturePath) {
    // Verificar si la textura ya está cargada
    if (persistentTextures.find(texturePath) != persistentTextures.end()) {
        if (debugMode) {
            std::cout << "Texture already loaded: " << texturePath << std::endl;
        }
        return;
    }
    
    // Cargar la textura
    auto texture = std::make_shared<Texture>();
    if (texture->loadFromFile(texturePath)) {
        persistentTextures[texturePath] = texture;
        if (debugMode) {
            std::cout << "Texture loaded successfully: " << texturePath << " (ID: " << texture->getID() << ")" << std::endl;
        }
    } else {
        if (debugMode) {
            std::cout << "Failed to load texture: " << texturePath << std::endl;
        }
    }
}

std::shared_ptr<Texture> SpriteAnimator::getTexture(const std::string& texturePath) {
    // Verificar si la textura está en el cache
    auto it = persistentTextures.find(texturePath);
    if (it != persistentTextures.end()) {
        return it->second;
    }
    
    // Si no está en el cache, cargarla
    loadTexture(texturePath);
    return persistentTextures[texturePath];
}

void SpriteAnimator::preloadAllTextures() {
    if (debugMode) {
        std::cout << "Preloading all textures..." << std::endl;
    }
    
    for (const auto& state : SpriteStates) {
        for (const auto& texturePath : state.texturePaths) {
            loadTexture(texturePath);
        }
    }
    
    if (debugMode) {
        std::cout << "Preloaded " << persistentTextures.size() << " textures" << std::endl;
    }
}

void SpriteAnimator::clearTextureCache() {
    if (debugMode) {
        std::cout << "Clearing texture cache (" << persistentTextures.size() << " textures)" << std::endl;
    }
    persistentTextures.clear();
}