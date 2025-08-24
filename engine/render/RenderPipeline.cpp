#include "RenderPipeline.h"
#include "Camera.h"
#include "DefaultShaders.h"
#include "../ui/Canvas.h"
#include "Material.h"
#include "MaterialManager.h"
#include "Light.h"
#include "Frustum.h"
#include "Framebuffer.h"
#include "AssimpGeometry.h"
#include "RenderConfig.h"
#include "ShadowManager.h"

#include "../components/GameObject.h"
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <algorithm>
#include <cstddef>
#include <direct.h>
#include <cstdio>
#include <errno.h>
#include <filesystem>
#include <ft2build.h>
#include FT_FREETYPE_H

RenderPipeline::RenderPipeline(Camera* cam, DefaultShaders* shd)
    : camera(cam), shaders(shd), targetFramebuffer(nullptr), usePBR(true), lowAmbient(false), ambientIntensity(1.0f),
      frustumCullingEnabled(true), shadowsEnabled(true), shadowManager(nullptr), visibleObjectsCount(0), totalObjectsCount(0), materialsDirty(false) {

    // FreeType is now handled by Canvas2D
    
    int initialWidth = 1920;
    int initialHeight = 1080;
    
    // Try to get actual window size from RenderConfig
    if (RenderConfig::getInstance().getWindow()) {
        int w, h;
        SDL_GetWindowSize(RenderConfig::getInstance().getWindow(), &w, &h);
        if (w > 0 && h > 0) {
            initialWidth = w;
            initialHeight = h;
        }
    }
    
    // Get current working directory for debugging
    char cwd[1024];
    if (_getcwd(cwd, sizeof(cwd)) != NULL) {
        // Current working directory available if needed
    }
    
    // Initialize shadow manager
    shadowManager = new ShadowManager();
    shadowManager->initialize(4096); // 4096x4096 shadow maps para mejor calidad
}

RenderPipeline::~RenderPipeline() {
    // Clean up shadow manager
    if (shadowManager) {
        delete shadowManager;
        shadowManager = nullptr;
    }
    
    for (auto canvas : _canvas) {
        delete canvas;
    }
    _canvas.clear();
}

void RenderPipeline::AddGameObject(GameObject* object) {
    sceneObjects.push_back(object);
}

void RenderPipeline::RemoveGameObject(GameObject* object) {
    if (object) {
        sceneObjects.erase(std::remove(sceneObjects.begin(), sceneObjects.end(), object), sceneObjects.end());
    }
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
    // 1. Shadow pass - render to shadow maps first
    if (shadowsEnabled) {
        renderShadowPass();
    }
    
    // 2. Main rendering pass
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

    shaders->getProgram()->use();

    glm::mat4 view = camera->getViewMatrix();
    glm::mat4 projection = camera->getProjectionMatrix();
    glm::vec3 cameraPos = camera->getPosition();

    shaders->getProgram()->setMat4("view", view);
    shaders->getProgram()->setMat4("projection", projection);
    shaders->getProgram()->setVec3("uViewPos", cameraPos);
    shaders->getProgram()->setBool("uUsePBR", usePBR);
    shaders->getProgram()->setInt("uUseModelNormals", 0);
    
    // CORREGIDO: Configurar inversión de normales (puedes cambiar esto a true si las normales siguen al revés)
    shaders->getProgram()->setBool("uFlipNormals", false);

    // Configurar iluminación
    configureLighting();
    
    // Configurar shadow mapping
    if (shadowsEnabled && shadowManager) {
        shadowManager->bindShadowMap(shaders->getProgram()->getID());
        shadowManager->setupShadowUniforms(shaders->getProgram()->getID());
        
        // Configurar shadow maps avanzados para spot y point lights
        std::vector<std::shared_ptr<Light>> spotLightsForShadows;
        std::vector<std::shared_ptr<Light>> pointLightsForShadows;
        
        // Recopilar spot y point lights para sombras
        for (auto& light : lights) {
            if (!light->isEnabled()) continue;
            
            if (light->getType() == LightType::Spot && spotLightsForShadows.size() < 2) {
                spotLightsForShadows.push_back(light);
            } else if (light->getType() == LightType::Point && pointLightsForShadows.size() < 4) {
                pointLightsForShadows.push_back(light);
            }
        }
        
        // Configurar shadow maps avanzados si tenemos luces spot o point
        if (!spotLightsForShadows.empty() || !pointLightsForShadows.empty()) {
            shadowManager->bindAllShadowMaps(shaders->getProgram()->getID());
            shadowManager->setupAllShadowUniforms(shaders->getProgram()->getID(), spotLightsForShadows, pointLightsForShadows);
        }
        
        // Enable shadows in shader
        GLint enableShadowsLoc = glGetUniformLocation(shaders->getProgram()->getID(), "uEnableShadows");
        glUniform1i(enableShadowsLoc, 1);
        
        // Habilitar sombras de spot y point lights SOLO SI HAY LUCES
        GLint enableSpotShadowsLoc = glGetUniformLocation(shaders->getProgram()->getID(), "uEnableSpotShadows");
        GLint enablePointShadowsLoc = glGetUniformLocation(shaders->getProgram()->getID(), "uEnablePointShadows");
        
        // Solo habilitar spot shadows si tenemos spot lights con shadow maps
        int spotShadowsEnabled = !spotLightsForShadows.empty() ? 1 : 0;
        glUniform1i(enableSpotShadowsLoc, spotShadowsEnabled);
        
        // Habilitar point shadows si hay point lights
        int pointShadowsEnabled = !pointLightsForShadows.empty() ? 1 : 0;
        glUniform1i(enablePointShadowsLoc, pointShadowsEnabled);
    } else {
        GLint enableShadowsLoc = shaders->getProgram()->getInt("uEnableShadows");
        GLint enableSpotShadowsLoc = shaders->getProgram()->getInt("uEnableSpotShadows");
        GLint enablePointShadowsLoc = shaders->getProgram()->getInt("uEnablePointShadows");
        glUniform1i(enableShadowsLoc, 0);
        glUniform1i(enableSpotShadowsLoc, 0);
        glUniform1i(enablePointShadowsLoc, 0);
    }

    renderInstanced();
    
    // Update canvas size to match camera buffer if available
    updateCanvasFromCameraBuffer();
    

    // Render all canvases
    for (auto canvas : _canvas) {
        if (canvas) {
            // Example: Draw some text on each canvas
            canvas->DrawElements();
        }
    }
    
    // Unbind framebuffer if specified
    if (activeFramebuffer) {
        activeFramebuffer->unbind();
    }
}

void RenderPipeline::configureLighting() {
    // Configurar luz ambiental - variable según el modo e intensidad
    glm::vec3 baseAmbient;
    if (lowAmbient) {
        baseAmbient = glm::vec3(0.1f, 0.1f, 0.15f); // Bajo para ver sombras
    } else {
        baseAmbient = glm::vec3(0.6f, 0.6f, 0.7f); // Alto para visibilidad
    }
    
    // Aplicar multiplicador de intensidad
    glm::vec3 finalAmbient = baseAmbient * ambientIntensity;
    shaders->getProgram()->setVec3("uAmbientLight", finalAmbient);
    
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
        shaders->getProgram()->setInt("uHasDirLight", 1);
        shaders->getProgram()->setVec3("uDirLightDirection", dirLight->getDirection());
        shaders->getProgram()->setVec3("uDirLightColor", dirLight->getColor());
        shaders->getProgram()->setFloat("uDirLightIntensity", dirLight->getIntensity());

    } else {
        shaders->getProgram()->setInt("uHasDirLight", 0);
    }
        
    shaders->getProgram()->setInt("uNumPointLights", static_cast<int>(pointLights.size()));

    // Iterar sobre las luces
    for (int i = 0; i < pointLights.size(); i++) {
        auto& light = pointLights[i];

        shaders->getProgram()->setVec3("uPointLightPositions[" + std::to_string(i) + "]", light->getPosition());
        shaders->getProgram()->setVec3("uPointLightColors[" + std::to_string(i) + "]", light->getColor());
        shaders->getProgram()->setFloat("uPointLightIntensities[" + std::to_string(i) + "]", light->getIntensity());
        shaders->getProgram()->setVec3("uPointLightAttenuations[" + std::to_string(i) + "]", light->getAttenuation());
        shaders->getProgram()->setFloat("uPointLightMinDistances[" + std::to_string(i) + "]", light->getMinDistance());
        shaders->getProgram()->setFloat("uPointLightMaxDistances[" + std::to_string(i) + "]", light->getMaxDistance());
    }
    
    shaders->getProgram()->setInt("uNumSpotLights", static_cast<int>(spotLights.size()));

    for (int i = 0; i < spotLights.size(); i++) {
        auto& light = spotLights[i];

        shaders->getProgram()->setVec3("uSpotLightPositions[" + std::to_string(i) + "]", light->getPosition());
        shaders->getProgram()->setVec3("uSpotLightDirections[" + std::to_string(i) + "]", light->getDirection());
        shaders->getProgram()->setVec3("uSpotLightColors[" + std::to_string(i) + "]", light->getColor());
        shaders->getProgram()->setFloat("uSpotLightIntensities[" + std::to_string(i) + "]", light->getIntensity());

        // Ángulos
        float cutOff = light->getCutOffAngle();
        float outerCutOff = light->getOuterCutOffAngle();

        if (outerCutOff < cutOff) {
            outerCutOff = cutOff + 0.1f;
        }

        shaders->getProgram()->setFloat("uSpotLightCutOffs[" + std::to_string(i) + "]", cutOff);
        shaders->getProgram()->setFloat("uSpotLightOuterCutOffs[" + std::to_string(i) + "]", outerCutOff);
        shaders->getProgram()->setFloat("uSpotLightRanges[" + std::to_string(i) + "]", light->getSpotRange());

        // Sombras
        if (shadowsEnabled && shadowManager) {
            const auto& spotMatrices = shadowManager->getSpotLightSpaceMatrices();
            if (i < spotMatrices.size()) {
                shaders->getProgram()->setMat4("uSpotLightMatrices[" + std::to_string(i) + "]", spotMatrices[i]);
            }
        }
    }
}

void RenderPipeline::renderInstanced() {
    // Resetear contadores
    visibleObjectsCount = 0;
    totalObjectsCount = sceneObjects.size();
    
    // Check if materials need refreshing
    if (materialsDirty) {
        materialsDirty = false;
    }
    
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
        
        // Validar que el objeto tenga un material válido
        auto material = obj->getMaterial();
        if (!material) {
            continue;
        }
        
        if (!material->isValid()) {
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
        
        // Configurar material y shadow maps en el orden correcto
        GLuint program = shaders->getProgram()->getID();
        
        // 1. Configurar material primero - ALWAYS configure material to ensure fresh state
        if (key.material) {
            configureMaterial(key.material.get());
        } else {
            configureDefaultMaterial();
        }
        
        // 2. Configurar shadow maps después del material
        if (shadowsEnabled && shadowManager) {
            shadowManager->bindAllShadowMaps(program);
            
            // Recopilar luces para shadow maps
            std::vector<std::shared_ptr<Light>> spotLightsForShadows;
            std::vector<std::shared_ptr<Light>> pointLightsForShadows;
            
            for (auto& light : lights) {
                if (!light->isEnabled()) continue;
                
                if (light->getType() == LightType::Spot && spotLightsForShadows.size() < 2) {
                    spotLightsForShadows.push_back(light);
                } else if (light->getType() == LightType::Point && pointLightsForShadows.size() < 4) {
                    pointLightsForShadows.push_back(light);
                }
            }
            
            shadowManager->setupAllShadowUniforms(program, spotLightsForShadows, pointLightsForShadows);
        }
        
        AssimpGeometry* geometry = key.geometry;
        
        // Configurar si usa normales de modelo
        glUniform1i(glGetUniformLocation(program, "uUseModelNormals"), geometry->usesModelNormals() ? 1 : 0);
        
        // Optimización: usar instanced rendering para grupos grandes
        if (objects.size() >= 2) {
            // Batch render múltiples objetos
            std::vector<glm::mat4> modelMatrices;
            modelMatrices.reserve(objects.size());
            
            for (GameObject* obj : objects) {
                modelMatrices.push_back(obj->getWorldModelMatrix());
            }
            
            // CORREGIDO: Asegurar que las texturas estén correctamente vinculadas antes del renderizado instanciado
            if (key.material && key.material->hasAnyValidTextures()) {
                key.material->bindTextures();
            }
            
            geometry->updateInstanceBuffer(modelMatrices);
            geometry->drawInstanced(modelMatrices);
        } else {
            // Renderizar individualmente si solo hay un objeto
            GameObject* obj = objects[0];
            glm::mat4 model = obj->getWorldModelMatrix();
            
            // CORREGIDO: Asegurar que las texturas estén correctamente vinculadas antes del renderizado instanciado
            if (key.material && key.material->hasAnyValidTextures()) {
                key.material->bindTextures();
            }
            
            std::vector<glm::mat4> singleInstance = { model };
            geometry->updateInstanceBuffer(singleInstance);
            geometry->drawInstanced(singleInstance);
        }
    }
}

void RenderPipeline::configureMaterial(Material* material) {
    if (!material) {
        std::cerr << "RenderPipeline::configureMaterial: ERROR - Material is nullptr!" << std::endl;
        configureDefaultMaterial();
        return;
    }

    if (!material->isValid()) {
        std::cerr << "RenderPipeline::configureMaterial: ERROR - Material '" 
                  << material->getName() << "' is not valid!" << std::endl;
        configureDefaultMaterial();
        return;
    }

    auto shader = shaders->getProgram();
    shader->use(); // Activar shader

    // Propiedades del material
    shader->setVec3("uAlbedo", material->getAlbedo());
    shader->setFloat("uAlpha", material->getAlpha());
    shader->setFloat("uMetallic", material->getMetallic());
    shader->setFloat("uRoughness", material->getRoughness());
    shader->setVec3("uEmissive", material->getEmissive());
    shader->setVec2("uTiling", material->getTiling());
    shader->setFloat("uNormalStrength", material->getNormalStrength());

    // Texturas
    shader->setInt("uAlbedoTexture", 0);
    shader->setInt("uNormalTexture", 1);
    shader->setInt("uMetallicTexture", 2);
    shader->setInt("uRoughnessTexture", 3);
    shader->setInt("uEmissiveTexture", 4);
    shader->setInt("uAOTexture", 5);

    // Flags de texturas
    shader->setInt("uHasAlbedoTexture", material->hasAlbedoTexture() ? 1 : 0);
    shader->setInt("uHasNormalTexture", material->hasNormalTexture() ? 1 : 0);
    shader->setInt("uHasMetallicTexture", material->hasMetallicTexture() ? 1 : 0);
    shader->setInt("uHasRoughnessTexture", material->hasRoughnessTexture() ? 1 : 0);
    shader->setInt("uHasEmissiveTexture", material->hasEmissiveTexture() ? 1 : 0);
    shader->setInt("uHasAOTexture", material->hasAOTexture() ? 1 : 0);

    // Bindear texturas del material
    if (material->hasAnyValidTextures()) {
        material->bindTextures();
    } else {
        // Resetear todas las unidades a 0 si no hay texturas
        for (int i = 0; i <= 5; i++) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        glActiveTexture(GL_TEXTURE0);
    }

    // Rebindear shadow maps después de las texturas
    // rebindShadowMapsAfterMaterial(shader->getProgram());
}

void RenderPipeline::configureDefaultMaterial() {
    auto shader = shaders->getProgram();
    shader->use(); // Activar shader

    // Propiedades del material por defecto
    shader->setVec3("uAlbedo", glm::vec3(1.0f, 1.0f, 1.0f));
    shader->setFloat("uAlpha", 1.0f);
    shader->setFloat("uMetallic", 0.0f);
    shader->setFloat("uRoughness", 0.5f);
    shader->setVec3("uEmissive", glm::vec3(0.0f));
    shader->setVec2("uTiling", glm::vec2(1.0f, 1.0f));
    shader->setFloat("uNormalStrength", 1.0f);

    // Flags de texturas desactivados
    shader->setInt("uHasAlbedoTexture", 0);
    shader->setInt("uHasNormalTexture", 0);
    shader->setInt("uHasMetallicTexture", 0);
    shader->setInt("uHasRoughnessTexture", 0);
    shader->setInt("uHasEmissiveTexture", 0);
    shader->setInt("uHasAOTexture", 0);

    // Resetear todas las unidades de textura
    for (int i = 0; i <= 5; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glActiveTexture(GL_TEXTURE0);

    // Rebindear shadow maps después del material por defecto
    // rebindShadowMapsAfterMaterial(shader->getProgram());
}

// Método helper para rebindear shadow maps después de cualquier configuración de material
void RenderPipeline::rebindShadowMapsAfterMaterial(GLuint program) {
    if (!shadowsEnabled || !shadowManager) return;
    
    std::cout << "RenderPipeline: CRÍTICO - Rebindeando shadow maps después de configurar material..." << std::endl;
    
    // Recopilar luces para shadow maps avanzados
    std::vector<std::shared_ptr<Light>> spotLightsForShadows;
    std::vector<std::shared_ptr<Light>> pointLightsForShadows;
    
    for (auto& light : lights) {
        if (!light->isEnabled()) continue;
        
        if (light->getType() == LightType::Spot && spotLightsForShadows.size() < 2) {
            spotLightsForShadows.push_back(light);
        } else if (light->getType() == LightType::Point && pointLightsForShadows.size() < 4) {
            pointLightsForShadows.push_back(light);
        }
    }
    
    // ESTRATEGIA AGRESIVA: Siempre rebindear TODOS los shadow maps Y uniforms
    shadowManager->bindAllShadowMaps(program);
    shadowManager->setupAllShadowUniforms(program, spotLightsForShadows, pointLightsForShadows);
    
    // También rebindear el shadow map básico por seguridad
    shadowManager->bindShadowMap(program);
    shadowManager->setupShadowUniforms(program);
    
    std::cout << "RenderPipeline: Shadow maps Y uniforms rebindeados completamente después de material" << std::endl;
}

void RenderPipeline::renderNonInstanced() {
    auto shader = shaders->getProgram();

    for (GameObject* obj : sceneObjects) {
        if (!obj->hasGeometry()) continue;

        glm::mat4 model = obj->getWorldModelMatrix();
        shader->setMat4("model", model); // Reemplaza glUniformMatrix4fv

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
        return it->second;
    }
    auto model = std::make_shared<AssimpGeometry>(path);
    
    if (model && model->isLoaded()) {
        // Guardar en cache solo si se cargó correctamente
        modelCache[path] = model;
        return model;
    } else {
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

Camera* RenderPipeline::getCamera() const {
    return camera;
}

void RenderPipeline::updateCanvasSize(int width, int height) {
    for (auto canvas : _canvas) {
        if (canvas) {
            // Check if camera has a framebuffer and use its size if available
            if (camera && camera->isFramebufferEnabled()) {
                auto [bufferWidth, bufferHeight] = camera->getBufferSize();
                if (bufferWidth > 0 && bufferHeight > 0) {
                    canvas->updateAspectRatio(bufferWidth, bufferHeight);
                    continue;
                }
            }
            // Fallback to window size if no camera buffer
            canvas->updateAspectRatio(width, height);
        }
    }
}


void RenderPipeline::updateCanvasFromCameraBuffer() {
    if (!camera) return;
    for (auto canvas : _canvas) {
        if (canvas && camera->isFramebufferEnabled()) {
            auto [bufferWidth, bufferHeight] = camera->getBufferSize();
            if (bufferWidth > 0 && bufferHeight > 0) {
                canvas->updateAspectRatio(bufferWidth, bufferHeight);
            }
        }
    }
}

Canvas2D* RenderPipeline::addCanvas(int width, int height) {
    Canvas2D* new_Canvas = nullptr;

    try {
        new_Canvas = new Canvas2D(width, height);
        
        const char* fontPaths[] = {
            "engine/fonts/Ubuntu-Regular.ttf",           // Relative to project root
            "./engine/fonts/Ubuntu-Regular.ttf",         // Current directory
            "../engine/fonts/Ubuntu-Regular.ttf",        // Parent directory  
            "C:/Users/tupap/source/repos/MantraxEngine/engine/fonts/Ubuntu-Regular.ttf",  // Fallback
            "fonts/Ubuntu-Regular.ttf"                   // Alternative path
        };

        bool fontLoaded = false;
        for (const char* path : fontPaths) {
            std::cout << "[RenderPipeline] Trying font path: " << path << std::endl;
            if (std::filesystem::exists(path)) {
                std::cout << "[RenderPipeline] Font file exists, loading..." << std::endl;
                if (new_Canvas->loadFont(path, 32)) {
                    std::cout << "[RenderPipeline] Font loaded successfully from: " << path << std::endl;
                    fontLoaded = true;
                    break;
                } else {
                    std::cout << "[RenderPipeline] Failed to load font from: " << path << std::endl;
                }
            } else {
                std::cout << "[RenderPipeline] Font file does not exist: " << path << std::endl;
            }
        }
        
        if (!fontLoaded) {
            std::cerr << "[RenderPipeline] WARNING: No font could be loaded! Text rendering may not work." << std::endl;
        }

        // Only add to vector if creation was successful
        _canvas.push_back(new_Canvas);
        return new_Canvas;
        
    } catch (const std::exception& e) {
        std::cerr << "Failed to create Canvas2D: " << e.what() << std::endl;
        if (new_Canvas) {
            delete new_Canvas;
        }
        return nullptr;
    } catch (...) {
        std::cerr << "Unknown error occurred while creating Canvas2D" << std::endl;
        if (new_Canvas) {
            delete new_Canvas;
        }
        return nullptr;
    }
}


void RenderPipeline::removeCanvas(size_t index) {
    if (index < _canvas.size()) {
        delete _canvas[index];
        _canvas.erase(_canvas.begin() + static_cast<std::ptrdiff_t>(index));
    }
}

Canvas2D* RenderPipeline::getCanvas(size_t index) {
    if (index < _canvas.size()) {
        return _canvas[index];
    }
    return nullptr;
}

size_t RenderPipeline::getCanvasCount() const {
    return _canvas.size();
}

// Shadow pass rendering
void RenderPipeline::renderShadowPass() {
    if (!shadowManager || !shadowManager->isInitialized() || !camera) {
        return;
    }
    
    // Find directional light for shadow casting (opcional)
    std::shared_ptr<Light> directionalLight = nullptr;
    for (const auto& light : lights) {
        if (light->isEnabled() && light->getType() == LightType::Directional) {
            directionalLight = light;
            break;
        }
    }

    // Recopilar luces spot y point para shadow rendering
    std::vector<std::shared_ptr<Light>> spotLightsForShadows;
    std::vector<std::shared_ptr<Light>> pointLightsForShadows;
    
    for (const auto& light : lights) {
        if (!light->isEnabled()) continue;
        
        if (light->getType() == LightType::Spot && spotLightsForShadows.size() < 2) {
            spotLightsForShadows.push_back(light);
        } else if (light->getType() == LightType::Point && pointLightsForShadows.size() < 4) {
            pointLightsForShadows.push_back(light);
        }
    }
    
    // Preparar spot y point shadow passes (calcular matrices) solo si hay luces
    bool hasSpotLights = !spotLightsForShadows.empty();
    bool hasPointLights = !pointLightsForShadows.empty();
    
    if (hasSpotLights) {
        shadowManager->beginSpotShadowPass(spotLightsForShadows, camera);
    }
    
    if (hasPointLights) {
        shadowManager->beginPointShadowPass(pointLightsForShadows, camera);
    }
    
    // 1. Render directional light shadow map (solo si hay directional light)
    if (directionalLight) {
        shadowManager->beginShadowPass(directionalLight, camera);
        renderShadowGeometry();
        shadowManager->endShadowPass();
    }
    
    // 2. Render spot light shadow maps (solo si hay spot lights)
    if (!spotLightsForShadows.empty()) {
        for (size_t i = 0; i < spotLightsForShadows.size() && i < 2; i++) {
            auto& light = spotLightsForShadows[i];
            glm::mat4 spotMatrix = shadowManager->getSpotLightSpaceMatrices()[i];
            
            shadowManager->beginSingleSpotShadowRender(i, spotMatrix);
            renderShadowGeometry();
            shadowManager->endSingleSpotShadowRender();
        }
    }
    
    // 3. Point light shadow maps - HABILITADO CON RENDIMIENTO OPTIMIZADO
    if (!pointLightsForShadows.empty()) {
        for (size_t i = 0; i < pointLightsForShadows.size() && i < 4; i++) {
            auto& light = pointLightsForShadows[i];
            glm::vec3 lightPos = light->getPosition();
            float farPlane = light->getMaxDistance();
            
            // Renderizar las 6 caras del cube map
            std::vector<glm::mat4> viewMatrices = shadowManager->calculatePointLightViewMatrices(lightPos);
            for (int face = 0; face < 6; face++) {
                glm::mat4 lightSpaceMatrix = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, farPlane) * viewMatrices[face];
                
                shadowManager->beginSinglePointShadowRender(i, face, lightSpaceMatrix);
                renderShadowGeometry();
                shadowManager->endSinglePointShadowRender();
            }
        }
    }
    
    // End shadow passes (solo si se iniciaron)
    if (hasSpotLights) {
        shadowManager->endSpotShadowPass();
    }
    if (hasPointLights) {
        shadowManager->endPointShadowPass();
    }
}

// Helper method for rendering geometry during shadow passes
void RenderPipeline::renderShadowGeometry() {
    // Render all objects from light's perspective
    std::map<MaterialGeometryKey, std::vector<GameObject*>> geometryGroups;
    int validObjects = 0;
    
    for (GameObject* obj : sceneObjects) {
        if (!obj->hasGeometry()) continue;
        
        MaterialGeometryKey key;
        key.material = obj->getMaterial();
        key.geometry = obj->getGeometry();
        
        geometryGroups[key].push_back(obj);
        validObjects++;
    }
    
    // Shadow pass rendering objects in groups
    
    // Render each geometry group with instanced rendering
    for (auto& group : geometryGroups) {
        auto& objects = group.second;
        AssimpGeometry* geometry = group.first.geometry;
        
        if (objects.size() > 1) {
            // Prepare model matrices for instanced rendering
            std::vector<glm::mat4> modelMatrices;
            for (GameObject* obj : objects) {
                modelMatrices.push_back(obj->getWorldModelMatrix());
            }
            
            geometry->updateInstanceBuffer(modelMatrices);
            geometry->drawInstanced(modelMatrices);
        } else if (!objects.empty()) {
            // Single object
            GameObject* obj = objects[0];
            glm::mat4 model = obj->getWorldModelMatrix();
            
            std::vector<glm::mat4> singleInstance = { model };
            geometry->updateInstanceBuffer(singleInstance);
            geometry->drawInstanced(singleInstance);
        }
    }
}

// Shadow mapping methods
void RenderPipeline::enableShadows(bool enabled) {
    shadowsEnabled = enabled;
}

bool RenderPipeline::getShadowsEnabled() const {
    return shadowsEnabled;
}

void RenderPipeline::setShadowMapSize(int size) {
    if (shadowManager) {
        shadowManager->cleanup();
        shadowManager->initialize(size);
    }
}

int RenderPipeline::getShadowMapSize() const {
    return shadowManager ? shadowManager->getShadowMapSize() : 0;
}

void RenderPipeline::setShadowBias(float bias) {
    if (shadowManager) {
        shadowManager->setShadowBias(bias);
    }
}

float RenderPipeline::getShadowBias() const {
    return shadowManager ? shadowManager->getShadowBias() : 0.0f;
}

void RenderPipeline::setShadowStrength(float strength) {
    if (shadowManager) {
        shadowManager->setShadowStrength(strength);
    }
}

float RenderPipeline::getShadowStrength() const {
    return shadowManager ? shadowManager->getShadowStrength() : 0.0f;
}

void RenderPipeline::forceMaterialRefresh() {
    materialsDirty = true;

    auto shader = shaders->getProgram();
    shader->use(); // Activar shader de forma consistente

    // Resetear todas las unidades de textura
    for (int i = 0; i < 6; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glActiveTexture(GL_TEXTURE0);
}

void RenderPipeline::markMaterialsDirty() {
    materialsDirty = true;
}

