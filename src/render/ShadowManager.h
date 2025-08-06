#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
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
    
    // Shadow rendering
    void beginShadowPass(std::shared_ptr<Light> directionalLight, Camera* camera);
    void endShadowPass();
    
    // Shader integration
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
    
private:
    bool initialized;
    int shadowMapSize;
    
    // OpenGL objects
    GLuint framebuffer;
    GLuint depthTexture;
    GLuint shadowShader;
    
    // Shadow parameters
    float shadowBias;
    float shadowStrength;
    glm::mat4 lightSpaceMatrix;
    
    // Current rendering state
    std::shared_ptr<Light> currentLight;
    Camera* currentCamera;
    
    // Private methods
    void createFramebuffer();
    void createShadowShader();
    glm::mat4 calculateLightSpaceMatrix(std::shared_ptr<Light> light, Camera* camera);
};