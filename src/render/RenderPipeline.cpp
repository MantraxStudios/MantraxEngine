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
      frustumCullingEnabled(true), shadowsEnabled(true), shadowManager(nullptr), visibleObjectsCount(0), totalObjectsCount(0) {

    // FreeType is now handled by Canvas2D
    std::cout << "FreeType will be initialized by Canvas2D" << std::endl;
    
    int initialWidth = 1920;
    int initialHeight = 1080;
    
    // Try to get actual window size from RenderConfig
    if (RenderConfig::getInstance().getWindow()) {
        int w, h;
        SDL_GetWindowSize(RenderConfig::getInstance().getWindow(), &w, &h);
        if (w > 0 && h > 0) {
            initialWidth = w;
            initialHeight = h;
            std::cout << "Canvas initialized with actual window size: " << w << "x" << h << std::endl;
        }
    }
    
    // Get current working directory for debugging
    char cwd[1024];
    if (_getcwd(cwd, sizeof(cwd)) != NULL) {
        std::cout << "Current working directory: " << cwd << std::endl;
    } else {
        std::cerr << "Failed to get current working directory" << std::endl;
    }
    
    // Initialize shadow manager
    shadowManager = new ShadowManager();
    shadowManager->initialize(4096); // 4096x4096 shadow maps para mejor calidad
    std::cout << "ShadowManager initialized successfully" << std::endl;
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
        std::cout << "RenderPipeline: Executing shadow pass (shadows enabled)" << std::endl;
        renderShadowPass();
    } else {
        std::cout << "RenderPipeline: Skipping shadow pass (shadows disabled)" << std::endl;
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
    
    // Configurar shadow mapping
    if (shadowsEnabled && shadowManager) {
        std::cout << "RenderPipeline: Setting up simple shadow uniforms..." << std::endl;
        shadowManager->bindShadowMap(program);
        shadowManager->setupShadowUniforms(program);
        
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
            shadowManager->bindAllShadowMaps(program);
            shadowManager->setupAllShadowUniforms(program, spotLightsForShadows, pointLightsForShadows);
        }
        
        // Enable shadows in shader
        GLint enableShadowsLoc = glGetUniformLocation(program, "uEnableShadows");
        glUniform1i(enableShadowsLoc, 1);
        std::cout << "RenderPipeline: uEnableShadows location: " << enableShadowsLoc << ", enabled" << std::endl;
        
        // Habilitar sombras de spot y point lights SOLO SI HAY LUCES
        GLint enableSpotShadowsLoc = glGetUniformLocation(program, "uEnableSpotShadows");
        GLint enablePointShadowsLoc = glGetUniformLocation(program, "uEnablePointShadows");
        
        // Solo habilitar spot shadows si tenemos spot lights con shadow maps
        int spotShadowsEnabled = !spotLightsForShadows.empty() ? 1 : 0;
        glUniform1i(enableSpotShadowsLoc, spotShadowsEnabled);
        
        // Habilitar point shadows si hay point lights
        int pointShadowsEnabled = !pointLightsForShadows.empty() ? 1 : 0;
        glUniform1i(enablePointShadowsLoc, pointShadowsEnabled);
        
        std::cout << "RenderPipeline: Spot shadows: " << (spotShadowsEnabled ? "enabled" : "disabled") 
                  << ", Point shadows: disabled (performance)" << std::endl;
    } else {
        GLint enableShadowsLoc = glGetUniformLocation(program, "uEnableShadows");
        GLint enableSpotShadowsLoc = glGetUniformLocation(program, "uEnableSpotShadows");
        GLint enablePointShadowsLoc = glGetUniformLocation(program, "uEnablePointShadows");
        glUniform1i(enableShadowsLoc, 0);
        glUniform1i(enableSpotShadowsLoc, 0);
        glUniform1i(enablePointShadowsLoc, 0);
        std::cout << "RenderPipeline: All shadows disabled in shader (shadowsEnabled: " << shadowsEnabled << ", shadowManager: " << (shadowManager ? "valid" : "null") << ")" << std::endl;
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
        
        // CORREGIDO: Usar las matrices de sombra calculadas por el ShadowManager
        if (shadowsEnabled && shadowManager) {
            // Usar la matriz de spot light calculada por el shadow manager
            const auto& spotMatrices = shadowManager->getSpotLightSpaceMatrices();
            if (i < spotMatrices.size()) {
                GLint spotMatrixLoc = glGetUniformLocation(program, ("uSpotLightMatrices[" + std::to_string(i) + "]").c_str());
                if (spotMatrixLoc != -1) {
                    glUniformMatrix4fv(spotMatrixLoc, 1, GL_FALSE, glm::value_ptr(spotMatrices[i]));
                    std::cout << "RenderPipeline: Set spot light " << i << " matrix (location: " << spotMatrixLoc << ")" << std::endl;
                } else {
                    std::cout << "RenderPipeline: WARNING - Could not find uSpotLightMatrices[" << i << "] uniform!" << std::endl;
                }
            } else {
                std::cout << "RenderPipeline: WARNING - No spot matrix available for light " << i << std::endl;
            }
        }
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
        
                // Configurar material y shadow maps en el orden correcto
        GLuint program = shaders->getProgram();
        
        // 1. Configurar material primero
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
    
    // Bind texturas del material
    material->bindTextures();
    
    // CRÍTICO: Rebindear TODOS los shadow maps después de bindear texturas del material
    rebindShadowMapsAfterMaterial(program);
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
    
    // CRÍTICO: También rebindear shadow maps después del material por defecto
    rebindShadowMapsAfterMaterial(program);
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
                    canvas->setUISize(bufferWidth, bufferHeight);
                    std::cout << "Canvas size updated to camera buffer size: "
                        << bufferWidth << "x" << bufferHeight << std::endl;
                    continue;
                }
            }
            // Fallback to window size if no camera buffer
            canvas->setUISize(width, height);
            std::cout << "Canvas size updated to window size: "
                << width << "x" << height << std::endl;
        }
    }
}


void RenderPipeline::updateCanvasFromCameraBuffer() {
    if (!camera) return;
    for (auto canvas : _canvas) {
        if (canvas && camera->isFramebufferEnabled()) {
            auto [bufferWidth, bufferHeight] = camera->getBufferSize();
            if (bufferWidth > 0 && bufferHeight > 0) {
                canvas->setUISize(bufferWidth, bufferHeight);
                std::cout << "Canvas updated to camera buffer size: "
                    << bufferWidth << "x" << bufferHeight << std::endl;
            }
        }
    }
}

Canvas2D* RenderPipeline::addCanvas(int width, int height) {
    Canvas2D* new_Canvas = nullptr;

    try {
        new_Canvas = new Canvas2D(width, height);
        
        const char* fontPaths[] = {
        "C:/Users/tupap/source/repos/MantraxEngine/engine/fonts/Ubuntu-Regular.ttf"
        };

        for (const char* path : fontPaths) {
            std::cout << "Trying path: " << path << std::endl;
            if (std::filesystem::exists(path)) {
                std::cout << "File exists: " << path << std::endl;
                if (new_Canvas->loadFont(path, 32)) {
                    std::cout << "Font loaded successfully from: " << path << std::endl;
                    break;
                }
            }
            else {
                std::cout << "File does not exist: " << path << std::endl;
            }
        }

        // Only add to vector if creation was successful
        _canvas.push_back(new_Canvas);
        std::cout << "Canvas added: " << width << "x" << height << " (Total: " << _canvas.size() << ")" << std::endl;
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
        std::cout << "Canvas removed at index " << index << " (Total: " << _canvas.size() << ")" << std::endl;
    }
    else {
        std::cerr << "Canvas index " << index << " out of range!" << std::endl;
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
        std::cout << "RenderPipeline: Cannot render shadows - shadowManager: " << (shadowManager ? "valid" : "null") 
                  << ", initialized: " << (shadowManager ? shadowManager->isInitialized() : false) 
                  << ", camera: " << (camera ? "valid" : "null") << std::endl;
        return;
    }
    
    std::cout << "RenderPipeline: Looking for lights (" << lights.size() << " total lights)" << std::endl;
    
    // Find directional light for shadow casting (opcional)
    std::shared_ptr<Light> directionalLight = nullptr;
    for (const auto& light : lights) {
        std::cout << "RenderPipeline: Checking light - Type: " << (int)light->getType() 
                  << ", Enabled: " << light->isEnabled() << std::endl;
        if (light->isEnabled() && light->getType() == LightType::Directional) {
            directionalLight = light;
            break;
        }
    }
    
    // CORREGIDO: Permitir spot light shadows aún sin directional light
    if (!directionalLight) {
        std::cout << "RenderPipeline: No directional light found, but continuing with spot/point lights" << std::endl;
    }
    
    if (directionalLight) {
        std::cout << "RenderPipeline: Found directional light for shadows" << std::endl;
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
        std::cout << "RenderPipeline: Initializing spot shadow pass for " << spotLightsForShadows.size() << " lights" << std::endl;
        shadowManager->beginSpotShadowPass(spotLightsForShadows, camera);
    }
    
    if (hasPointLights) {
        std::cout << "RenderPipeline: Initializing point shadow pass for " << pointLightsForShadows.size() << " lights" << std::endl;
        shadowManager->beginPointShadowPass(pointLightsForShadows, camera);
    }
    
    // 1. Render directional light shadow map (solo si hay directional light)
    if (directionalLight) {
        std::cout << "RenderPipeline: Rendering directional light shadow map" << std::endl;
        shadowManager->beginShadowPass(directionalLight, camera);
        renderShadowGeometry();
        shadowManager->endShadowPass();
    } else {
        std::cout << "RenderPipeline: Skipping directional light shadow map (no directional light)" << std::endl;
    }
    
    // 2. Render spot light shadow maps (solo si hay spot lights)
    if (!spotLightsForShadows.empty()) {
        std::cout << "RenderPipeline: Rendering " << spotLightsForShadows.size() << " spot light shadow maps" << std::endl;
        for (size_t i = 0; i < spotLightsForShadows.size() && i < 2; i++) {
            auto& light = spotLightsForShadows[i];
            glm::mat4 spotMatrix = shadowManager->getSpotLightSpaceMatrices()[i];
            
            std::cout << "RenderPipeline: Rendering spot light " << i << " shadow map" << std::endl;
            shadowManager->beginSingleSpotShadowRender(i, spotMatrix);
            renderShadowGeometry();
            shadowManager->endSingleSpotShadowRender();
        }
    } else {
        std::cout << "RenderPipeline: No spot lights for shadows, skipping spot shadow maps" << std::endl;
    }
    
    // 3. Point light shadow maps - HABILITADO CON RENDIMIENTO OPTIMIZADO
    if (!pointLightsForShadows.empty()) {
        std::cout << "RenderPipeline: Rendering " << pointLightsForShadows.size() << " point light shadow maps" << std::endl;
        for (size_t i = 0; i < pointLightsForShadows.size() && i < 4; i++) {
            auto& light = pointLightsForShadows[i];
            glm::vec3 lightPos = light->getPosition();
            float farPlane = light->getMaxDistance();
            
            // Renderizar las 6 caras del cube map
            std::vector<glm::mat4> viewMatrices = shadowManager->calculatePointLightViewMatrices(lightPos);
            for (int face = 0; face < 6; face++) {
                glm::mat4 lightSpaceMatrix = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, farPlane) * viewMatrices[face];
                
                std::cout << "RenderPipeline: Rendering point light " << i << " face " << face << " shadow map" << std::endl;
                shadowManager->beginSinglePointShadowRender(i, face, lightSpaceMatrix);
                renderShadowGeometry();
                shadowManager->endSinglePointShadowRender();
            }
        }
    } else {
        std::cout << "RenderPipeline: No point lights for shadows, skipping point shadow maps" << std::endl;
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
    
    std::cout << "RenderPipeline: Shadow pass rendering " << validObjects << " objects in " << geometryGroups.size() << " groups" << std::endl;
    
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
            std::cout << "RenderPipeline: Shadow pass rendered " << objects.size() << " instanced objects" << std::endl;
        } else if (!objects.empty()) {
            // Single object
            GameObject* obj = objects[0];
            glm::mat4 model = obj->getWorldModelMatrix();
            
            std::vector<glm::mat4> singleInstance = { model };
            geometry->updateInstanceBuffer(singleInstance);
            geometry->drawInstanced(singleInstance);
            std::cout << "RenderPipeline: Shadow pass rendered 1 object" << std::endl;
        }
    }
}

// Shadow mapping methods
void RenderPipeline::enableShadows(bool enabled) {
    shadowsEnabled = enabled;
    std::cout << "Shadows " << (enabled ? "enabled" : "disabled") << std::endl;
}

bool RenderPipeline::getShadowsEnabled() const {
    return shadowsEnabled;
}

void RenderPipeline::setShadowMapSize(int size) {
    if (shadowManager) {
        shadowManager->cleanup();
        shadowManager->initialize(size);
        std::cout << "Shadow map size set to " << size << "x" << size << std::endl;
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

