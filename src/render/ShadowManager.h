#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <vector>
#include "../core/CoreExporter.h"
#include "Light.h"
#include "Camera.h"

class MANTRAXCORE_API ShadowManager {
public:
    ShadowManager();
    ~ShadowManager();

    // Core functions
    void initialize(int shadowMapSize = 2048);
    void cleanup();
    
    // Shadow rendering - Directional Light
    void beginDirectionalShadowPass(std::shared_ptr<Light> directionalLight, Camera* camera);
    void endDirectionalShadowPass();
    
    // Shadow rendering - Spot Lights
    void beginSpotShadowPass(const std::vector<std::shared_ptr<Light>>& spotLights, Camera* camera);
    void endSpotShadowPass();
    void beginSingleSpotShadowRender(int lightIndex, const glm::mat4& lightSpaceMatrix);
    void endSingleSpotShadowRender();
    
    // Shadow rendering - Point Lights
    void beginPointShadowPass(const std::vector<std::shared_ptr<Light>>& pointLights, Camera* camera);
    void endPointShadowPass();
    void beginSinglePointShadowRender(int lightIndex, int faceIndex, const glm::mat4& lightSpaceMatrix);
    void endSinglePointShadowRender();
    
    // Shader integration
    void bindAllShadowMaps(GLuint shaderProgram);
    void setupAllShadowUniforms(GLuint shaderProgram, const std::vector<std::shared_ptr<Light>>& spotLights, const std::vector<std::shared_ptr<Light>>& pointLights);
    
    // Legacy methods for backward compatibility
    void beginShadowPass(std::shared_ptr<Light> directionalLight, Camera* camera);
    void endShadowPass();
    void bindShadowMap(GLuint shaderProgram);
    void setupShadowUniforms(GLuint shaderProgram);
    
    // Configuration
    void setShadowMapSize(int size);
    void setShadowBias(float bias) { shadowBias = bias; }
    void setShadowStrength(float strength) { shadowStrength = strength; }
    
    // Getters
    int getShadowMapSize() const { return shadowMapSize; }
    float getShadowBias() const { return shadowBias; }
    float getShadowStrength() const { return shadowStrength; }
    GLuint getShadowShader() const { return shadowShader; }
    glm::mat4 getLightSpaceMatrix() const { return lightSpaceMatrix; }
    bool isInitialized() const { return initialized; }
    
    // New getters for advanced shadow maps
    const std::vector<glm::mat4>& getSpotLightSpaceMatrices() const { return spotLightSpaceMatrices; }
    
    // Light space matrix calculations (public access needed for rendering)
    glm::mat4 calculateDirectionalLightSpaceMatrix(std::shared_ptr<Light> light, Camera* camera);
    glm::mat4 calculateSpotLightSpaceMatrix(std::shared_ptr<Light> light);
    std::vector<glm::mat4> calculatePointLightViewMatrices(glm::vec3 lightPos);
    
private:
    bool initialized;
    int shadowMapSize;
    
    // Directional Light Shadow Mapping
    GLuint dirFramebuffer;
    GLuint dirDepthTexture;
    glm::mat4 lightSpaceMatrix;
    
    // Spot Light Shadow Mapping (up to 2 spot lights)
    GLuint spotFramebuffers[2];
    GLuint spotDepthTextures[2];
    std::vector<glm::mat4> spotLightSpaceMatrices;
    
    // Point Light Shadow Mapping (up to 4 point lights)
    GLuint pointFramebuffers[4];
    GLuint pointDepthCubeMaps[4];
    std::vector<float> pointLightFarPlanes;
    
    // Shader
    GLuint shadowShader;
    
    // Shadow parameters
    float shadowBias;
    float shadowStrength;
    
    // Current rendering state
    std::shared_ptr<Light> currentLight;
    Camera* currentCamera;
    int currentSpotIndex;
    int currentPointIndex;
    int currentCubeMapFace;
    
    // Private methods
    void createDirectionalFramebuffer();
    void createSpotFramebuffers();
    void createPointFramebuffers();
    void createShadowShader();
    
    // Cleanup helpers
    void cleanupDirectionalShadows();
    void cleanupSpotShadows();
    void cleanupPointShadows();
};