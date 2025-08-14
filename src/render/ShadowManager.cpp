#include "ShadowManager.h"
#include "Camera.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <limits>
#include <vector>

#include "../core/FileSystem.h"

ShadowManager::ShadowManager() 
    : initialized(false)
    , shadowMapSize(4096)
    , dirFramebuffer(0)
    , dirDepthTexture(0)
    , shadowShader(0)
    , shadowBias(0.001f)
    , shadowStrength(0.8f)
    , lightSpaceMatrix(1.0f)
    , currentLight(nullptr)
    , currentCamera(nullptr)
    , currentSpotIndex(-1)
    , currentPointIndex(-1)
    , currentCubeMapFace(0) {
    
    // Initialize spot light arrays
    for (int i = 0; i < 2; i++) {
        spotFramebuffers[i] = 0;
        spotDepthTextures[i] = 0;
    }
    
    // Initialize point light arrays
    for (int i = 0; i < 4; i++) {
        pointFramebuffers[i] = 0;
        pointDepthCubeMaps[i] = 0;
    }
}

ShadowManager::~ShadowManager() {
    cleanup();
}

void ShadowManager::initialize(int size) {
    if (initialized) cleanup();
    
    shadowMapSize = size;
    std::cout << "ShadowManager: Initializing complete shadow system (size: " << shadowMapSize << ")" << std::endl;
    
    createDirectionalFramebuffer();
    createSpotFramebuffers();
    createPointFramebuffers();
    createShadowShader();
    
    initialized = true;
    std::cout << "ShadowManager: Complete shadow system initialized successfully" << std::endl;
}

void ShadowManager::cleanup() {
    if (!initialized) return;
    
    cleanupDirectionalShadows();
    cleanupSpotShadows();
    cleanupPointShadows();
    
    if (shadowShader) {
        glDeleteProgram(shadowShader);
        shadowShader = 0;
    }
    
    initialized = false;
    std::cout << "ShadowManager: Complete shadow system cleaned up" << std::endl;
}

void ShadowManager::createDirectionalFramebuffer() {
    // Generate framebuffer
    glGenFramebuffers(1, &dirFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, dirFramebuffer);
    
    // Generate depth texture
    glGenTextures(1, &dirDepthTexture);
    glBindTexture(GL_TEXTURE_2D, dirDepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, shadowMapSize, shadowMapSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    
    // Set texture parameters for shadow mapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    
    // Set border color to white (no shadow)
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    
    // Configure for shadow comparison
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    
    // Attach depth texture as FBO's depth buffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, dirDepthTexture, 0);
    
    // No color buffer needed
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowManager::createSpotFramebuffers() {
    for (int i = 0; i < 2; i++) {
        // Generate framebuffer
        glGenFramebuffers(1, &spotFramebuffers[i]);
        glBindFramebuffer(GL_FRAMEBUFFER, spotFramebuffers[i]);

        // Generate depth texture
        glGenTextures(1, &spotDepthTextures[i]);
        glBindTexture(GL_TEXTURE_2D, spotDepthTextures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, shadowMapSize, shadowMapSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

        // Set texture parameters for shadow mapping
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

        // Set border color to white (no shadow)
        float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

        // Configure for shadow comparison
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

        // Attach depth texture as FBO's depth buffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, spotDepthTextures[i], 0);

        // No color buffer needed
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void ShadowManager::createPointFramebuffers() {
    for (int i = 0; i < 4; i++) {
        // Generate framebuffer
        glGenFramebuffers(1, &pointFramebuffers[i]);
        glBindFramebuffer(GL_FRAMEBUFFER, pointFramebuffers[i]);
        
        // Generate depth cube map
        glGenTextures(1, &pointDepthCubeMaps[i]);
        glBindTexture(GL_TEXTURE_CUBE_MAP, pointDepthCubeMaps[i]);
        
        // Create 6 faces of the cube map
        for (int face = 0; face < 6; face++) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_DEPTH_COMPONENT24, 
                        shadowMapSize, shadowMapSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        }
        
        // Set texture parameters for shadow mapping
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        
        // Configure for shadow comparison
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
        
        // No color buffer needed
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowManager::createShadowShader() {

    std::string _Vert;
    std::string _Frag;

    FileSystem::readString("engine/shaders/ShadowVert.glsl", _Vert);
    FileSystem::readString("engine/shaders/ShadowFrag.glsl", _Frag);
    const char* vertSource = _Vert.c_str();
    const char* fragSource = _Frag.c_str();

    // Create vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertSource, nullptr);
    glCompileShader(vertexShader);
    
    // Check compilation
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
    }
    
    // Create fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragSource, nullptr);
    glCompileShader(fragmentShader);
    
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
    }
    
    // Create shader program
    shadowShader = glCreateProgram();
    glAttachShader(shadowShader, vertexShader);
    glAttachShader(shadowShader, fragmentShader);
    glLinkProgram(shadowShader);
    
    glGetProgramiv(shadowShader, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shadowShader, 512, nullptr, infoLog);
    }
    
    // Clean up shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void ShadowManager::beginShadowPass(std::shared_ptr<Light> directionalLight, Camera* camera) {
    // Delegate to new method for backward compatibility
    beginDirectionalShadowPass(directionalLight, camera);
}

void ShadowManager::endShadowPass() {
    // Delegate to new method for backward compatibility
    endDirectionalShadowPass();
}

void ShadowManager::bindShadowMap(GLuint shaderProgram) {
    if (!initialized) return;
    
    // CRÍTICO: Forzar rebinding del shadow map direccional
    glActiveTexture(GL_TEXTURE10);
    glBindTexture(GL_TEXTURE_2D, dirDepthTexture);
    
    // Set shadow map uniform
    GLint shadowMapLoc = glGetUniformLocation(shaderProgram, "uShadowMap");
    glUniform1i(shadowMapLoc, 10);
    
    // CRÍTICO: Restaurar unidad de textura activa
    glActiveTexture(GL_TEXTURE0);
}

void ShadowManager::setupShadowUniforms(GLuint shaderProgram) {
    if (!initialized) return;
    
    // Set light space matrix
    GLint lightSpaceMatrixLoc = glGetUniformLocation(shaderProgram, "uLightSpaceMatrix");
    glUniformMatrix4fv(lightSpaceMatrixLoc, 1, GL_FALSE, &lightSpaceMatrix[0][0]);
    
    // Set shadow parameters
    GLint shadowBiasLoc = glGetUniformLocation(shaderProgram, "uShadowBias");
    GLint shadowStrengthLoc = glGetUniformLocation(shaderProgram, "uShadowStrength");
    GLint enableShadowsLoc = glGetUniformLocation(shaderProgram, "uEnableShadows");
    
    glUniform1f(shadowBiasLoc, shadowBias);
    glUniform1f(shadowStrengthLoc, shadowStrength);
    glUniform1i(enableShadowsLoc, 1);
}

glm::mat4 ShadowManager::calculateDirectionalLightSpaceMatrix(std::shared_ptr<Light> light, Camera* camera) {
    // Get light direction - asegurarse de que est� normalizada
    glm::vec3 lightDir = glm::normalize(light->getDirection());

    // Get camera properties
    glm::vec3 cameraPos = camera->getPosition();
    glm::vec3 cameraForward = camera->getForward();
    glm::vec3 cameraUp = camera->getUp();
    glm::vec3 cameraRight = camera->getRight();

    // Calculate camera frustum bounds - usar distancias m�s razonables para sombras
    float fov = camera->getFOV();
    float aspect = camera->getAspectRatio();
    float nearDist = camera->getNearClip();
    // CORREGIDO: Distancia de sombras adaptiva basada en la cámara
    float maxShadowDistance = 20.0f; // Reducir aún más para mejor resolución
    float farDist = std::min(camera->getFarClip(), maxShadowDistance);

    // Calculate frustum dimensions
    float nearHeight = 2.0f * tan(glm::radians(fov) * 0.5f) * nearDist;
    float nearWidth = nearHeight * aspect;
    float farHeight = 2.0f * tan(glm::radians(fov) * 0.5f) * farDist;
    float farWidth = farHeight * aspect;

    // Calculate frustum corners in world space
    glm::vec3 nearCenter = cameraPos + cameraForward * nearDist;
    glm::vec3 farCenter = cameraPos + cameraForward * farDist;

    std::vector<glm::vec3> frustumCorners = {
        // Near plane corners
        nearCenter + cameraUp * (nearHeight * 0.5f) - cameraRight * (nearWidth * 0.5f),
        nearCenter + cameraUp * (nearHeight * 0.5f) + cameraRight * (nearWidth * 0.5f),
        nearCenter - cameraUp * (nearHeight * 0.5f) - cameraRight * (nearWidth * 0.5f),
        nearCenter - cameraUp * (nearHeight * 0.5f) + cameraRight * (nearWidth * 0.5f),
        // Far plane corners
        farCenter + cameraUp * (farHeight * 0.5f) - cameraRight * (farWidth * 0.5f),
        farCenter + cameraUp * (farHeight * 0.5f) + cameraRight * (farWidth * 0.5f),
        farCenter - cameraUp * (farHeight * 0.5f) - cameraRight * (farWidth * 0.5f),
        farCenter - cameraUp * (farHeight * 0.5f) + cameraRight * (farWidth * 0.5f)
    };

    // PROBLEMA PRINCIPAL: Mejorar el c�lculo del sistema de coordenadas de luz
    glm::vec3 lightUp;

    // Elegir un vector up que no sea paralelo a lightDir
    if (abs(lightDir.y) < 0.9f) {
        lightUp = glm::vec3(0.0f, 1.0f, 0.0f); // Usar Y como up si no es paralelo
    }
    else {
        lightUp = glm::vec3(1.0f, 0.0f, 0.0f); // Usar X si Y es paralelo
    }

    // Crear sistema de coordenadas ortogonal para la luz
    glm::vec3 lightRight = glm::normalize(glm::cross(lightUp, lightDir));
    lightUp = glm::normalize(glm::cross(lightDir, lightRight));

    // Encontrar el centro del frustum para posicionar la luz
    glm::vec3 frustumCenter = glm::vec3(0.0f);
    for (const auto& corner : frustumCorners) {
        frustumCenter += corner;
    }
    frustumCenter /= frustumCorners.size();

    // Posicionar la luz lejos del centro del frustum en la direcci�n opuesta
    float lightDistance = 100.0f; // Distancia fija para luz direccional
    glm::vec3 lightPosition = frustumCenter - lightDir * lightDistance;

    // Create light view matrix - CORREGIR: usar lightPosition como origen
    glm::mat4 lightView = glm::lookAt(lightPosition, lightPosition + lightDir, lightUp);

    // Transform frustum corners to light space para encontrar bounds
    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();
    float minZ = std::numeric_limits<float>::max();
    float maxZ = std::numeric_limits<float>::lowest();

    for (const auto& corner : frustumCorners) {
        glm::vec4 lightSpaceCorner = lightView * glm::vec4(corner, 1.0f);
        minX = std::min(minX, lightSpaceCorner.x);
        maxX = std::max(maxX, lightSpaceCorner.x);
        minY = std::min(minY, lightSpaceCorner.y);
        maxY = std::max(maxY, lightSpaceCorner.y);
        // CORREGIDO: Usar Z directamente (ya está en light space)
        minZ = std::min(minZ, lightSpaceCorner.z);
        maxZ = std::max(maxZ, lightSpaceCorner.z);
    }

    // CORREGIR: Extender hacia atr�s de manera m�s controlada
    float backExtension = maxShadowDistance * 0.5f; // Reducir extensión hacia atrás // Extensi�n fija hacia atr�s
    minZ -= backExtension; // Extender hacia atr�s para capturar shadow casters

    // CORREGIR: Padding m�s peque�o y sim�trico
    float padding = 1.0f; // Menos padding para mejor resolución
    minX -= padding;
    maxX += padding;
    minY -= padding;
    maxY += padding;

    // Asegurar que tenemos volumen v�lido
    if (maxX - minX < 1.0f) {
        float center = (maxX + minX) * 0.5f;
        minX = center - 0.5f;
        maxX = center + 0.5f;
    }
    if (maxY - minY < 1.0f) {
        float center = (maxY + minY) * 0.5f;
        minY = center - 0.5f;
        maxY = center + 0.5f;
    }
    if (maxZ - minZ < 1.0f) {
        maxZ = minZ + 1.0f;
    }

    // Create orthographic projection matrix - CORREGIR orden de par�metros
    // glm::ortho(left, right, bottom, top, near, far)
    // CRITICO: Para shadow mapping, necesitamos parametros correctos
    // Los valores Z negativos en view space se manejan correctamente
    // CORREGIDO: Para shadow mapping, near debe ser positivo y menor que far
    float nearPlane = -maxZ; // Convertir a distancia positiva
    float farPlane = -minZ;   // Convertir a distancia positiva
    
    // Asegurar que near < far y ambos son positivos
    if (nearPlane <= 0.0f) nearPlane = 0.1f;
    if (farPlane <= nearPlane) farPlane = nearPlane + 100.0f;
    
    glm::mat4 lightProjection = glm::ortho(minX, maxX, minY, maxY, nearPlane, farPlane);

    std::cout << "ShadowManager: Light space bounds - X: [" << minX << ", " << maxX
        << "], Y: [" << minY << ", " << maxY
        << "], Z: [" << minZ << ", " << maxZ << "]" << std::endl;
    std::cout << "ShadowManager: Near/Far planes: " << nearPlane << " / " << farPlane << std::endl;
    std::cout << "ShadowManager: Light position: (" << lightPosition.x << ", " << lightPosition.y << ", " << lightPosition.z << ")" << std::endl;
    std::cout << "ShadowManager: Light direction: (" << lightDir.x << ", " << lightDir.y << ", " << lightDir.z << ")" << std::endl;
    std::cout << "ShadowManager: Frustum center: (" << frustumCenter.x << ", " << frustumCenter.y << ", " << frustumCenter.z << ")" << std::endl;

    return lightProjection * lightView;
}

void ShadowManager::setShadowMapSize(int size) {
    if (size != shadowMapSize) {
        shadowMapSize = size;
        if (initialized) {
            std::cout << "ShadowManager: Updating shadow map size to " << size << "x" << size << std::endl;
            
            // Recreate all framebuffers with new size
            cleanupDirectionalShadows();
            cleanupSpotShadows();
            cleanupPointShadows();
            
            createDirectionalFramebuffer();
            createSpotFramebuffers();
            createPointFramebuffers();
        }
    }
}

// Cleanup methods
void ShadowManager::cleanupDirectionalShadows() {
    if (dirDepthTexture) {
        glDeleteTextures(1, &dirDepthTexture);
        dirDepthTexture = 0;
    }
    
    if (dirFramebuffer) {
        glDeleteFramebuffers(1, &dirFramebuffer);
        dirFramebuffer = 0;
    }
}

void ShadowManager::cleanupSpotShadows() {
    for (int i = 0; i < 2; i++) {
        if (spotDepthTextures[i]) {
            glDeleteTextures(1, &spotDepthTextures[i]);
            spotDepthTextures[i] = 0;
        }
        
        if (spotFramebuffers[i]) {
            glDeleteFramebuffers(1, &spotFramebuffers[i]);
            spotFramebuffers[i] = 0;
        }
    }
    spotLightSpaceMatrices.clear();
}

void ShadowManager::cleanupPointShadows() {
    for (int i = 0; i < 4; i++) {
        if (pointDepthCubeMaps[i]) {
            glDeleteTextures(1, &pointDepthCubeMaps[i]);
            pointDepthCubeMaps[i] = 0;
        }
        
        if (pointFramebuffers[i]) {
            glDeleteFramebuffers(1, &pointFramebuffers[i]);
            pointFramebuffers[i] = 0;
        }
    }
    pointLightFarPlanes.clear();
}

// New shadow pass methods implementation
void ShadowManager::beginDirectionalShadowPass(std::shared_ptr<Light> directionalLight, Camera* camera) {
    if (!initialized || !directionalLight || !camera) {
        std::cout << "ShadowManager: Cannot begin directional shadow pass - not initialized or missing parameters" << std::endl;
        return;
    }
    
    currentLight = directionalLight;
    currentCamera = camera;
    
    // Calculate light space matrix
    lightSpaceMatrix = calculateDirectionalLightSpaceMatrix(directionalLight, camera);
    
    // Bind shadow framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, dirFramebuffer);
    glViewport(0, 0, shadowMapSize, shadowMapSize);
    glClear(GL_DEPTH_BUFFER_BIT);
    
    // Use shadow shader
    glUseProgram(shadowShader);
    
    // Set light space matrix uniform
    GLint lightSpaceMatrixLoc = glGetUniformLocation(shadowShader, "uLightSpaceMatrix");
    glUniformMatrix4fv(lightSpaceMatrixLoc, 1, GL_FALSE, &lightSpaceMatrix[0][0]);
    
    // Enable depth testing and disable color writing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    
    // CORREGIDO: Disable culling para renderizar sombras desde ambas caras (como Unreal)
    // Esto permite que objetos delgados como hojas, vallas, etc. proyecten sombras correctamente
    glDisable(GL_CULL_FACE);
    
    std::cout << "ShadowManager: Directional shadow pass started" << std::endl;
}

void ShadowManager::endDirectionalShadowPass() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glEnable(GL_CULL_FACE); // Re-enable culling
    glCullFace(GL_BACK); // Restore normal culling
    glDepthFunc(GL_LESS);
    std::cout << "ShadowManager: Directional shadow pass ended" << std::endl;
}

// Spot Light Shadow Pass - IMPLEMENTACIÓN SIMPLIFICADA (solo calcula matrices)
void ShadowManager::beginSpotShadowPass(const std::vector<std::shared_ptr<Light>>& spotLights, Camera* camera) {
    if (!initialized || !camera || spotLights.empty()) {
        std::cout << "ShadowManager: Cannot begin spot shadow pass - not initialized or missing parameters" << std::endl;
        return;
    }
    
    currentCamera = camera;
    spotLightSpaceMatrices.clear();
    
    // Solo calcular matrices, el renderizado se hace con beginSingleSpotShadowRender()
    for (size_t i = 0; i < std::min(spotLights.size(), size_t(2)); i++) {
        glm::mat4 spotMatrix = calculateSpotLightSpaceMatrix(spotLights[i]);
        spotLightSpaceMatrices.push_back(spotMatrix);
    }
    
    std::cout << "ShadowManager: Spot shadow pass initialized for " << spotLightSpaceMatrices.size() << " lights" << std::endl;
}

// Nuevo método para renderizar individual spot light shadow map
void ShadowManager::beginSingleSpotShadowRender(int lightIndex, const glm::mat4& lightSpaceMatrix) {
    if (!initialized || lightIndex < 0 || lightIndex >= 2) {
        std::cout << "ShadowManager: Invalid spot light index: " << lightIndex << std::endl;
        return;
    }
    
    std::cout << "ShadowManager: Starting individual spot light " << lightIndex << " shadow render" << std::endl;
    
    // Bind framebuffer for this spot light
    glBindFramebuffer(GL_FRAMEBUFFER, spotFramebuffers[lightIndex]);
    glViewport(0, 0, shadowMapSize, shadowMapSize);
    glClear(GL_DEPTH_BUFFER_BIT);
    
    // Verificar que el framebuffer está completo
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "ShadowManager: ERROR - Spot framebuffer " << lightIndex << " not complete! Status: " << status << std::endl;
        return;
    }
    
    // Use shadow shader
    glUseProgram(shadowShader);
    
    // Set light space matrix uniform
    GLint lightSpaceMatrixLoc = glGetUniformLocation(shadowShader, "uLightSpaceMatrix");
    glUniformMatrix4fv(lightSpaceMatrixLoc, 1, GL_FALSE, &lightSpaceMatrix[0][0]);
    
    // Configure rendering state for shadows
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE); // Renderizar ambas caras
    
    currentSpotIndex = lightIndex;
}

void ShadowManager::endSingleSpotShadowRender() {
    std::cout << "ShadowManager: Ended individual spot light " << currentSpotIndex << " shadow render" << std::endl;
    currentSpotIndex = -1;
}

void ShadowManager::endSpotShadowPass() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glEnable(GL_CULL_FACE); // Re-enable culling
    glCullFace(GL_BACK);
    glDepthFunc(GL_LESS);
    currentSpotIndex = -1;
    std::cout << "ShadowManager: Spot shadow pass ended" << std::endl;
}

// Point Light Shadow Pass - IMPLEMENTACIÓN SIMPLIFICADA (solo calcula datos)
void ShadowManager::beginPointShadowPass(const std::vector<std::shared_ptr<Light>>& pointLights, Camera* camera) {
    if (!initialized || !camera || pointLights.empty()) {
        std::cout << "ShadowManager: Cannot begin point shadow pass - not initialized or missing parameters" << std::endl;
        return;
    }
    
    currentCamera = camera;
    pointLightFarPlanes.clear();
    
    // Calculate far planes for each point light
    for (size_t i = 0; i < std::min(pointLights.size(), size_t(4)); i++) {
        float farPlane = pointLights[i]->getMaxDistance(); // Use light's max distance as far plane
        pointLightFarPlanes.push_back(farPlane);
    }
    
    std::cout << "ShadowManager: Point shadow pass initialized for " << pointLightFarPlanes.size() << " lights" << std::endl;
}

// Nuevo método para renderizar individual point light shadow map face
void ShadowManager::beginSinglePointShadowRender(int lightIndex, int faceIndex, const glm::mat4& lightSpaceMatrix) {
    if (!initialized || lightIndex < 0 || lightIndex >= 4 || faceIndex < 0 || faceIndex >= 6) {
        std::cout << "ShadowManager: Invalid point light index: " << lightIndex << " or face: " << faceIndex << std::endl;
        return;
    }
    
    std::cout << "ShadowManager: Starting point light " << lightIndex << " face " << faceIndex << " shadow render" << std::endl;
    
    // Bind framebuffer for this point light
    glBindFramebuffer(GL_FRAMEBUFFER, pointFramebuffers[lightIndex]);
    
    // Attach specific cube map face to framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 
                          GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceIndex, 
                          pointDepthCubeMaps[lightIndex], 0);
    
    glViewport(0, 0, shadowMapSize, shadowMapSize);
    glClear(GL_DEPTH_BUFFER_BIT);
    
    // Use shadow shader
    glUseProgram(shadowShader);
    
    // Set light space matrix uniform
    GLint lightSpaceMatrixLoc = glGetUniformLocation(shadowShader, "uLightSpaceMatrix");
    glUniformMatrix4fv(lightSpaceMatrixLoc, 1, GL_FALSE, &lightSpaceMatrix[0][0]);
    
    // Configure rendering state for shadows
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE); // Renderizar ambas caras
    
    currentPointIndex = lightIndex;
    currentCubeMapFace = faceIndex;
}

void ShadowManager::endSinglePointShadowRender() {
    std::cout << "ShadowManager: Ended point light " << currentPointIndex << " face " << currentCubeMapFace << " shadow render" << std::endl;
}

void ShadowManager::endPointShadowPass() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glEnable(GL_CULL_FACE); // Re-enable culling
    glCullFace(GL_BACK);
    glDepthFunc(GL_LESS);
    currentPointIndex = -1;
    currentCubeMapFace = 0;
    std::cout << "ShadowManager: Point shadow pass ended" << std::endl;
}

// Matrix calculation method for spot lights
glm::mat4 ShadowManager::calculateSpotLightSpaceMatrix(std::shared_ptr<Light> light) {
    glm::vec3 lightPos = light->getPosition();
    glm::vec3 lightDir = glm::normalize(light->getDirection());
    
    // Calcular un vector up que no sea paralelo a la dirección de la luz
    glm::vec3 up;
    if (abs(lightDir.y) < 0.99f) {
        up = glm::vec3(0.0f, 1.0f, 0.0f);
    } else {
        up = glm::vec3(0.0f, 0.0f, 1.0f);
    }
    
    // Crear un sistema de coordenadas ortogonal
    glm::vec3 right = glm::normalize(glm::cross(lightDir, up));
    up = glm::normalize(glm::cross(right, lightDir));
    
    // Matriz de vista
    glm::mat4 lightView = glm::lookAt(lightPos, lightPos + lightDir, up);
    
    // Configurar proyección perspectiva para el spot light
    float outerCutOff = light->getOuterCutOffAngle();
    float fov = glm::degrees(outerCutOff * 2.0f); // Convertir a grados
    float aspect = 1.0f; // Shadow map cuadrado
    float nearPlane = 0.1f;
    float farPlane = light->getSpotRange();
    
    // Ajustar near y far planes para mejor precisión
    nearPlane = glm::max(nearPlane, farPlane * 0.001f);
    farPlane = glm::min(farPlane, 100.0f); // Limitar el rango máximo
    
    glm::mat4 lightProjection = glm::perspective(fov, aspect, nearPlane, farPlane);
    
    // Aplicar un pequeño bias a la matriz para evitar self-shadowing
    glm::mat4 biasMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -0.001f));
    
    return lightProjection * biasMatrix * lightView;
}

// Point light view matrices calculation
std::vector<glm::mat4> ShadowManager::calculatePointLightViewMatrices(glm::vec3 lightPos) {
    std::vector<glm::mat4> viewMatrices;
    
    // Six view matrices for cube map faces
    viewMatrices.push_back(glm::lookAt(lightPos, lightPos + glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f))); // +X
    viewMatrices.push_back(glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f))); // -X
    viewMatrices.push_back(glm::lookAt(lightPos, lightPos + glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f))); // +Y
    viewMatrices.push_back(glm::lookAt(lightPos, lightPos + glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f))); // -Y
    viewMatrices.push_back(glm::lookAt(lightPos, lightPos + glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f))); // +Z
    viewMatrices.push_back(glm::lookAt(lightPos, lightPos + glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))); // -Z
    
    return viewMatrices;
}

// Advanced shadow map binding and uniform setup methods
void ShadowManager::bindAllShadowMaps(GLuint shaderProgram) {
    if (!initialized) {
        std::cout << "ShadowManager: Cannot bind shadow maps - not initialized" << std::endl;
        return;
    }
    
    // Verificar que el shader program es válido
    GLint currentProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
    if (currentProgram != shaderProgram) {
        std::cout << "ShadowManager: WARNING - Shader program " << shaderProgram << " is not active (current: " << currentProgram << ")" << std::endl;
    }
    
    // 1. Bind directional shadow map to texture unit 10
    glActiveTexture(GL_TEXTURE10);
    glBindTexture(GL_TEXTURE_2D, dirDepthTexture);
    GLint shadowMapLoc = glGetUniformLocation(shaderProgram, "uShadowMap");
    if (shadowMapLoc != -1) {
        glUniform1i(shadowMapLoc, 10);
        std::cout << "ShadowManager: Bound directional shadow map to texture unit 10 (texture: " << dirDepthTexture << ", location: " << shadowMapLoc << ")" << std::endl;
    }
    
    // 2. Bind spot shadow maps to texture units 11-12
    for (int i = 0; i < 2; i++) {
        // Verificar que la textura existe
        if (spotDepthTextures[i] == 0) {
            continue;
        }
        
        // Bind textura y verificar errores
        glActiveTexture(GL_TEXTURE11 + i);
        glBindTexture(GL_TEXTURE_2D, spotDepthTextures[i]);
        GLenum bindError = glGetError();
        if (bindError != GL_NO_ERROR) {
            std::cout << "ShadowManager: ERROR - Failed to bind spot shadow map " << i << " (GL error: " << bindError << ")" << std::endl;
            continue;
        }
        
        // Configurar uniform y verificar errores
        std::string uniformName = "uSpotShadowMaps[" + std::to_string(i) + "]";
        GLint spotShadowMapLoc = glGetUniformLocation(shaderProgram, uniformName.c_str());
        if (spotShadowMapLoc != -1) {
            glUniform1i(spotShadowMapLoc, 11 + i);
            GLenum uniformError = glGetError();
            if (uniformError == GL_NO_ERROR) {
                std::cout << "ShadowManager: Successfully bound spot shadow map " << i << " to texture unit " << (11 + i) 
                          << " (texture: " << spotDepthTextures[i] << ", location: " << spotShadowMapLoc << ")" << std::endl;
            }
        }
    }
    
    // 3. Bind point shadow maps to texture units 13-16 (deshabilitado temporalmente)
    for (int i = 0; i < 4; i++) {
        if (pointDepthCubeMaps[i] == 0) {
            continue;
        }
        
        glActiveTexture(GL_TEXTURE13 + i);
        glBindTexture(GL_TEXTURE_CUBE_MAP, pointDepthCubeMaps[i]);
        std::string uniformName = "uPointShadowMaps[" + std::to_string(i) + "]";
        GLint pointShadowMapLoc = glGetUniformLocation(shaderProgram, uniformName.c_str());
        if (pointShadowMapLoc != -1) {
            glUniform1i(pointShadowMapLoc, 13 + i);
        }
    }
    
    // CRÍTICO: Restaurar unidad de textura activa
    glActiveTexture(GL_TEXTURE0);
    
    // Verificar estado final
    GLenum finalError = glGetError();
    if (finalError == GL_NO_ERROR) {
        //std::cout << "ShadowManager: All shadow maps bound successfully" << std::endl;
    } else {
        std::cout << "ShadowManager: WARNING - GL errors occurred during shadow map binding (last error: " << finalError << ")" << std::endl;
    }
}

void ShadowManager::setupAllShadowUniforms(GLuint shaderProgram, const std::vector<std::shared_ptr<Light>>& spotLights, const std::vector<std::shared_ptr<Light>>& pointLights) {
    if (!initialized) return;
    
    // Setup directional shadow uniforms
    GLint lightSpaceMatrixLoc = glGetUniformLocation(shaderProgram, "uLightSpaceMatrix");
    glUniformMatrix4fv(lightSpaceMatrixLoc, 1, GL_FALSE, &lightSpaceMatrix[0][0]);
    
    GLint shadowBiasLoc = glGetUniformLocation(shaderProgram, "uShadowBias");
    GLint shadowStrengthLoc = glGetUniformLocation(shaderProgram, "uShadowStrength");
    GLint enableShadowsLoc = glGetUniformLocation(shaderProgram, "uEnableShadows");
    GLint enableSpotShadowsLoc = glGetUniformLocation(shaderProgram, "uEnableSpotShadows");
    GLint enablePointShadowsLoc = glGetUniformLocation(shaderProgram, "uEnablePointShadows");
    
    glUniform1f(shadowBiasLoc, shadowBias);
    glUniform1f(shadowStrengthLoc, shadowStrength);
    glUniform1i(enableShadowsLoc, 1);
    glUniform1i(enableSpotShadowsLoc, !spotLights.empty() ? 1 : 0);
    glUniform1i(enablePointShadowsLoc, !pointLights.empty() ? 1 : 0);
    
    // Setup spot light space matrices
    for (size_t i = 0; i < std::min(spotLightSpaceMatrices.size(), size_t(2)); i++) {
        GLint spotMatrixLoc = glGetUniformLocation(shaderProgram, ("uSpotLightMatrices[" + std::to_string(i) + "]").c_str());
        if (spotMatrixLoc != -1) {
            glUniformMatrix4fv(spotMatrixLoc, 1, GL_FALSE, &spotLightSpaceMatrices[i][0][0]);
            std::cout << "ShadowManager: Set spot light space matrix " << i << " (location: " << spotMatrixLoc << ")" << std::endl;
        } else {
            std::cout << "ShadowManager: WARNING - spot light space matrix " << i << " uniform not found!" << std::endl;
        }
    }
    
    // Setup point light far planes
    for (size_t i = 0; i < std::min(pointLightFarPlanes.size(), size_t(4)); i++) {
        GLint farPlaneLoc = glGetUniformLocation(shaderProgram, ("uPointShadowFarPlanes[" + std::to_string(i) + "]").c_str());
        glUniform1f(farPlaneLoc, pointLightFarPlanes[i]);
    }
}