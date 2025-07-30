#include "RenderPipeline.h"
#include "Camera.h"
#include "DefaultShaders.h"
#include "Material.h"
#include "MaterialManager.h"
#include "Light.h"
#include "Frustum.h"
#include "Framebuffer.h"
#include "AssimpGeometry.h"

#include "../components/GameObject.h"
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

RenderPipeline::RenderPipeline(Camera* cam, DefaultShaders* shd)
    : camera(cam), shaders(shd), targetFramebuffer(nullptr), usePBR(true), lowAmbient(false), ambientIntensity(1.0f),
      frustumCullingEnabled(true), visibleObjectsCount(0), totalObjectsCount(0) {
}

RenderPipeline::~RenderPipeline() {
}

void RenderPipeline::AddGameObject(GameObject* object) {
    sceneObjects.push_back(object);
}

void RenderPipeline::AddLight(std::shared_ptr<Light> light) {
    lights.push_back(light);
}

void RenderPipeline::RemoveLight(std::shared_ptr<Light> light) {
    lights.erase(std::remove(lights.begin(), lights.end(), light), lights.end());
}

void RenderPipeline::clearGameObjects() {
    sceneObjects.clear();
    totalObjectsCount = 0;
    visibleObjectsCount = 0;
}

void RenderPipeline::clearLights() {
    lights.clear();
}

void RenderPipeline::setUsePBR(bool enabled) {
    usePBR = enabled;
}

bool RenderPipeline::getUsePBR() const {
    return usePBR;
}

void RenderPipeline::setLowAmbient(bool enabled) {
    lowAmbient = enabled;
}

bool RenderPipeline::getLowAmbient() const {
    return lowAmbient;
}

void RenderPipeline::setAmbientIntensity(float intensity) {
    ambientIntensity = glm::clamp(intensity, 0.0f, 2.0f); // Límite entre 0 y 2
}

float RenderPipeline::getAmbientIntensity() const {
    return ambientIntensity;
}

int RenderPipeline::getVisibleObjectsCount() const {
    return visibleObjectsCount;
}

void RenderPipeline::renderFrame() {
    // Check if camera has framebuffer enabled, prioritize it over targetFramebuffer
    Framebuffer* activeFramebuffer = nullptr;
    if (camera && camera->isFramebufferEnabled() && camera->getFramebuffer()) {
        activeFramebuffer = camera->getFramebuffer();
    } else if (targetFramebuffer) {
        activeFramebuffer = targetFramebuffer;
    }
    
    // Bind framebuffer if specified
    if (activeFramebuffer) {
        activeFramebuffer->bind();
    }
    
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    GLuint program = shaders->getProgram();
    glUseProgram(program);

    glm::mat4 view = camera->getViewMatrix();
    glm::mat4 projection = camera->getProjectionMatrix();

    // Configurar matrices de vista y proyección
    glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // Configurar posición de la cámara para lighting
    glm::vec3 cameraPos = camera->getPosition();
    glUniform3fv(glGetUniformLocation(program, "uViewPos"), 1, glm::value_ptr(cameraPos));

    // Configurar flag PBR/Blinn-Phong
    glUniform1i(glGetUniformLocation(program, "uUsePBR"), usePBR ? 1 : 0);
    
    // Configurar flag para normales de modelo (por defecto false para cubos)
    glUniform1i(glGetUniformLocation(program, "uUseModelNormals"), 0);

    // Configurar iluminación
    configureLighting();

    renderInstanced();
    
    // Unbind framebuffer if specified
    if (activeFramebuffer) {
        activeFramebuffer->unbind();
    }
}

void RenderPipeline::configureLighting() {
    GLuint program = shaders->getProgram();
    
    // Configurar luz ambiental - variable según el modo e intensidad
    glm::vec3 baseAmbient;
    if (lowAmbient) {
        baseAmbient = glm::vec3(0.1f, 0.1f, 0.15f); // Bajo para ver sombras
    } else {
        baseAmbient = glm::vec3(0.6f, 0.6f, 0.7f); // Alto para visibilidad
    }
    
    // Aplicar multiplicador de intensidad
    glm::vec3 finalAmbient = baseAmbient * ambientIntensity;
    glUniform3fv(glGetUniformLocation(program, "uAmbientLight"), 1, glm::value_ptr(finalAmbient));
    
    // Separar luces por tipo
    std::vector<std::shared_ptr<Light>> directionalLights;
    std::vector<std::shared_ptr<Light>> pointLights;
    std::vector<std::shared_ptr<Light>> spotLights;
    
    for (auto& light : lights) {
        if (!light->isEnabled()) continue;
        
        switch (light->getType()) {
            case LightType::Directional:
                directionalLights.push_back(light);
                break;
            case LightType::Point:
                if (pointLights.size() < 4) pointLights.push_back(light);
                break;
            case LightType::Spot:
                if (spotLights.size() < 2) spotLights.push_back(light);
                break;
        }
    }
    
    // Configurar luz direccional
    if (!directionalLights.empty()) {
        auto& dirLight = directionalLights[0];
        glUniform1i(glGetUniformLocation(program, "uHasDirLight"), 1);
        glUniform3fv(glGetUniformLocation(program, "uDirLightDirection"), 1, glm::value_ptr(dirLight->getDirection()));
        glUniform3fv(glGetUniformLocation(program, "uDirLightColor"), 1, glm::value_ptr(dirLight->getColor()));
        glUniform1f(glGetUniformLocation(program, "uDirLightIntensity"), dirLight->getIntensity());
    } else {
        glUniform1i(glGetUniformLocation(program, "uHasDirLight"), 0);
    }
    
    // Configurar luces puntuales
    glUniform1i(glGetUniformLocation(program, "uNumPointLights"), static_cast<int>(pointLights.size()));
    for (int i = 0; i < pointLights.size(); i++) {
        auto& light = pointLights[i];
        glUniform3fv(glGetUniformLocation(program, ("uPointLightPositions[" + std::to_string(i) + "]").c_str()), 1, glm::value_ptr(light->getPosition()));
        glUniform3fv(glGetUniformLocation(program, ("uPointLightColors[" + std::to_string(i) + "]").c_str()), 1, glm::value_ptr(light->getColor()));
        glUniform1f(glGetUniformLocation(program, ("uPointLightIntensities[" + std::to_string(i) + "]").c_str()), light->getIntensity());
        glUniform3fv(glGetUniformLocation(program, ("uPointLightAttenuations[" + std::to_string(i) + "]").c_str()), 1, glm::value_ptr(light->getAttenuation()));
        glUniform1f(glGetUniformLocation(program, ("uPointLightMinDistances[" + std::to_string(i) + "]").c_str()), light->getMinDistance());
        glUniform1f(glGetUniformLocation(program, ("uPointLightMaxDistances[" + std::to_string(i) + "]").c_str()), light->getMaxDistance());
    }
    
    // Configurar luces spot
    glUniform1i(glGetUniformLocation(program, "uNumSpotLights"), static_cast<int>(spotLights.size()));
    for (int i = 0; i < spotLights.size(); i++) {
        auto& light = spotLights[i];
        glUniform3fv(glGetUniformLocation(program, ("uSpotLightPositions[" + std::to_string(i) + "]").c_str()), 1, glm::value_ptr(light->getPosition()));
        glUniform3fv(glGetUniformLocation(program, ("uSpotLightDirections[" + std::to_string(i) + "]").c_str()), 1, glm::value_ptr(light->getDirection()));
        glUniform3fv(glGetUniformLocation(program, ("uSpotLightColors[" + std::to_string(i) + "]").c_str()), 1, glm::value_ptr(light->getColor()));
        glUniform1f(glGetUniformLocation(program, ("uSpotLightIntensities[" + std::to_string(i) + "]").c_str()), light->getIntensity());
        
        // Los ángulos ya vienen en radianes desde la clase Light
        float cutOff = light->getCutOffAngle();
        float outerCutOff = light->getOuterCutOffAngle();
        
        // Asegurarnos de que outer sea siempre mayor que inner
        if (outerCutOff < cutOff) {
            outerCutOff = cutOff + 0.1f;
        }
        
        glUniform1f(glGetUniformLocation(program, ("uSpotLightCutOffs[" + std::to_string(i) + "]").c_str()), cutOff);
        glUniform1f(glGetUniformLocation(program, ("uSpotLightOuterCutOffs[" + std::to_string(i) + "]").c_str()), outerCutOff);
        glUniform1f(glGetUniformLocation(program, ("uSpotLightRanges[" + std::to_string(i) + "]").c_str()), light->getSpotRange());
    }
}

void RenderPipeline::renderInstanced() {
    // Resetear contadores
    visibleObjectsCount = 0;
    totalObjectsCount = sceneObjects.size();
    
    // Obtener el frustum de la cámara SOLO si culling está habilitado
    Frustum cameraFrustum;
    if (frustumCullingEnabled) {
        cameraFrustum = camera->getFrustum();
    }
    
    // Agrupar objetos por material y geometría (solo objetos visibles)
    std::map<MaterialGeometryKey, std::vector<GameObject*>> materialGeometryGroups;
    
    for (GameObject* obj : sceneObjects) {
        // Skip objects without geometry
        if (!obj->hasGeometry()) {
            continue;
        }
        
        // Realizar frustum culling
        if (isObjectVisible(obj, cameraFrustum)) {
            visibleObjectsCount++;
            
            MaterialGeometryKey key;
            key.material = obj->getMaterial();
            key.geometry = obj->getGeometry();
            materialGeometryGroups[key].push_back(obj);
        }
    }
    
    // Renderizar cada grupo con instanced rendering optimizado
    for (auto& group : materialGeometryGroups) {
        const MaterialGeometryKey& key = group.first;
        std::vector<GameObject*>& objects = group.second;
        
        // Skip empty groups
        if (objects.empty()) continue;
        
        // Configurar material
        if (key.material) {
            configureMaterial(key.material.get());
        } else {
            configureDefaultMaterial();
        }
        
        		AssimpGeometry* geometry = key.geometry;
        
        // Configurar si usa normales de modelo
        GLuint program = shaders->getProgram();
        glUniform1i(glGetUniformLocation(program, "uUseModelNormals"), geometry->usesModelNormals() ? 1 : 0);
        
        // Optimización: usar instanced rendering para grupos grandes
        if (objects.size() >= 2) {
            // Batch render múltiples objetos
            std::vector<glm::mat4> modelMatrices;
            modelMatrices.reserve(objects.size());
            
            for (GameObject* obj : objects) {
                modelMatrices.push_back(obj->getWorldModelMatrix());
            }
            
            geometry->updateInstanceBuffer(modelMatrices);
            geometry->drawInstanced(modelMatrices);
        } else {
            // Renderizar individualmente si solo hay un objeto
            GameObject* obj = objects[0];
            glm::mat4 model = obj->getWorldModelMatrix();
            
            std::vector<glm::mat4> singleInstance = { model };
            geometry->updateInstanceBuffer(singleInstance);
            geometry->drawInstanced(singleInstance);
        }
    }
}

void RenderPipeline::configureMaterial(Material* material) {
    GLuint program = shaders->getProgram();
    
    // Configurar propiedades del material
    glUniform3fv(glGetUniformLocation(program, "uAlbedo"), 1, glm::value_ptr(material->getAlbedo()));
    glUniform1f(glGetUniformLocation(program, "uMetallic"), material->getMetallic());
    glUniform1f(glGetUniformLocation(program, "uRoughness"), material->getRoughness());
    glUniform3fv(glGetUniformLocation(program, "uEmissive"), 1, glm::value_ptr(material->getEmissive()));
    glUniform2fv(glGetUniformLocation(program, "uTiling"), 1, glm::value_ptr(material->getTiling()));
    glUniform1f(glGetUniformLocation(program, "uNormalStrength"), material->getNormalStrength());
    
    // Configurar texturas
    glUniform1i(glGetUniformLocation(program, "uAlbedoTexture"), 0);
    glUniform1i(glGetUniformLocation(program, "uNormalTexture"), 1);
    glUniform1i(glGetUniformLocation(program, "uMetallicTexture"), 2);
    glUniform1i(glGetUniformLocation(program, "uRoughnessTexture"), 3);
    glUniform1i(glGetUniformLocation(program, "uEmissiveTexture"), 4);
    glUniform1i(glGetUniformLocation(program, "uAOTexture"), 5);
    
    // Configurar flags de texturas
    glUniform1i(glGetUniformLocation(program, "uHasAlbedoTexture"), material->hasAlbedoTexture());
    glUniform1i(glGetUniformLocation(program, "uHasNormalTexture"), material->hasNormalTexture());
    glUniform1i(glGetUniformLocation(program, "uHasMetallicTexture"), material->hasMetallicTexture());
    glUniform1i(glGetUniformLocation(program, "uHasRoughnessTexture"), material->hasRoughnessTexture());
    glUniform1i(glGetUniformLocation(program, "uHasEmissiveTexture"), material->hasEmissiveTexture());
    glUniform1i(glGetUniformLocation(program, "uHasAOTexture"), material->hasAOTexture());
    
    // Bind texturas
    material->bindTextures();
}

void RenderPipeline::configureDefaultMaterial() {
    GLuint program = shaders->getProgram();
    
    // Material por defecto
    glUniform3f(glGetUniformLocation(program, "uAlbedo"), 1.0f, 1.0f, 1.0f);
    glUniform1f(glGetUniformLocation(program, "uMetallic"), 0.0f);
    glUniform1f(glGetUniformLocation(program, "uRoughness"), 0.5f);
    glUniform3f(glGetUniformLocation(program, "uEmissive"), 0.0f, 0.0f, 0.0f);
    glUniform2f(glGetUniformLocation(program, "uTiling"), 1.0f, 1.0f);
    glUniform1f(glGetUniformLocation(program, "uNormalStrength"), 1.0f);
    
    // Sin texturas
    glUniform1i(glGetUniformLocation(program, "uHasAlbedoTexture"), 0);
    glUniform1i(glGetUniformLocation(program, "uHasNormalTexture"), 0);
    glUniform1i(glGetUniformLocation(program, "uHasMetallicTexture"), 0);
    glUniform1i(glGetUniformLocation(program, "uHasRoughnessTexture"), 0);
    glUniform1i(glGetUniformLocation(program, "uHasEmissiveTexture"), 0);
    glUniform1i(glGetUniformLocation(program, "uHasAOTexture"), 0);
}

void RenderPipeline::renderNonInstanced() {
    // Método de respaldo para renderizado no instanciado (mantenido por compatibilidad)
    for (GameObject* obj : sceneObjects) {
        // Skip objects without geometry
        if (!obj->hasGeometry()) {
            continue;
        }
        
        glm::mat4 model = obj->getWorldModelMatrix();
        glUniformMatrix4fv(glGetUniformLocation(shaders->getProgram(), "model"), 1, GL_FALSE, glm::value_ptr(model));
        obj->getGeometry()->draw();
    }
}

// Implementaciones de frustum culling
void RenderPipeline::setFrustumCulling(bool enabled) {
    frustumCullingEnabled = enabled;
}

bool RenderPipeline::getFrustumCulling() const {
    return frustumCullingEnabled;
}

int RenderPipeline::getTotalObjectsCount() const {
    return totalObjectsCount;
}

void RenderPipeline::renderToFramebuffer(Framebuffer* framebuffer) {
    if (!framebuffer || !framebuffer->isValid()) {
        return;
    }
    
    // Store current target framebuffer
    Framebuffer* previousTarget = targetFramebuffer;
    
    // Set new target and render
    targetFramebuffer = framebuffer;
    renderFrame();
    
    // Restore previous target
    targetFramebuffer = previousTarget;
}

void RenderPipeline::renderToScreen() {
    // Store camera framebuffer state
    bool cameraFramebufferWasEnabled = false;
    if (camera && camera->isFramebufferEnabled()) {
        cameraFramebufferWasEnabled = true;
        camera->enableFramebuffer(false); // Temporarily disable
    }
    
    // Store current target framebuffer
    Framebuffer* previousTarget = targetFramebuffer;
    targetFramebuffer = nullptr; // Force render to screen
    
    // Render to screen
    renderFrame();
    
    // Restore states
    targetFramebuffer = previousTarget;
    if (cameraFramebufferWasEnabled && camera) {
        camera->enableFramebuffer(true); // Re-enable if it was enabled
    }
}

bool RenderPipeline::isObjectVisible(GameObject* object, const Frustum& cameraFrustum) const {
    if (!frustumCullingEnabled) {
        return true; // Si culling está deshabilitado, todos los objetos son visibles
    }
    
    // OPTIMIZADO: Usar bounding sphere en lugar de AABB (mucho más rápido)
    BoundingSphere worldSphere = object->getWorldBoundingSphere();
    
    // Realizar el test de frustum culling con sphere (más eficiente)
    CullResult result = cameraFrustum.testBoundingSphere(worldSphere);
    
    // Considerar visibles tanto los objetos completamente dentro como los que intersectan
    return result != CullResult::OUTSIDE;
}

// Resource Management Implementation

std::shared_ptr<AssimpGeometry> RenderPipeline::loadModel(const std::string& path) {
    // Verificar si ya está en cache
    auto it = modelCache.find(path);
    if (it != modelCache.end()) {
        std::cout << "Model loaded from cache: " << path << std::endl;
        return it->second;
    }
    
    // Cargar modelo nuevo
    std::cout << "Loading new model: " << path << std::endl;
    auto model = std::make_shared<AssimpGeometry>(path);
    
    if (model && model->isLoaded()) {
        // Guardar en cache solo si se cargó correctamente
        modelCache[path] = model;
        std::cout << "Model loaded and cached by RenderPipeline: " << path << std::endl;
        return model;
    } else {
        std::cerr << "RenderPipeline failed to load model: " << path << std::endl;
        return nullptr;
    }
}

std::shared_ptr<AssimpGeometry> RenderPipeline::getModel(const std::string& path) {
    auto it = modelCache.find(path);
    if (it != modelCache.end()) {
        return it->second;
    }
    return nullptr;
}



std::shared_ptr<Material> RenderPipeline::createMaterial(const glm::vec3& albedo, const std::string& name) {
    auto material = std::make_shared<Material>(name);
    material->setAlbedo(albedo);
    materialPool.push_back(material);
    std::cout << "Material '" << name << "' created by RenderPipeline (Total: " << materialPool.size() << ")" << std::endl;
    return material;
}

std::shared_ptr<Material> RenderPipeline::createMaterial(const glm::vec3& albedo) {
    return createMaterial(albedo, "Material");
}

std::shared_ptr<Material> RenderPipeline::createMaterial() {
    return createMaterial(glm::vec3(1.0f), "Material");
}

void RenderPipeline::clearModelCache() {
    modelCache.clear();
    std::cout << "RenderPipeline model cache cleared" << std::endl;
}

void RenderPipeline::listLoadedModels() const {
    std::cout << "=== RenderPipeline Resources ===" << std::endl;
    std::cout << "Models (" << modelCache.size() << "):" << std::endl;
    for (const auto& pair : modelCache) {
        const auto& model = pair.second;
        std::cout << "  - " << pair.first 
                  << " (Vertices: " << model->getVertexCount() 
                  << ", Indices: " << model->getIndexCount() << ")" << std::endl;
    }
    
    std::cout << "Materials: " << materialPool.size() << std::endl;
}

size_t RenderPipeline::getModelCacheSize() const {
    return modelCache.size();
}

// Material Management Implementation

bool RenderPipeline::loadMaterialsFromConfig(const std::string& configPath) {
    return MaterialManager::getInstance().loadMaterialsFromConfig(configPath);
}

std::shared_ptr<Material> RenderPipeline::getMaterial(const std::string& materialName) {
    return MaterialManager::getInstance().getMaterial(materialName);
}

bool RenderPipeline::hasMaterial(const std::string& materialName) const {
    return MaterialManager::getInstance().hasMaterial(materialName);
}

void RenderPipeline::listMaterials() const {
    MaterialManager::getInstance().listMaterials();
}
