#include "ShadowManager.h"
#include "Camera.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <limits>
#include <vector>

ShadowManager::ShadowManager() 
    : initialized(false)
    , shadowMapSize(4096)
    , framebuffer(0)
    , depthTexture(0)
    , shadowShader(0)
    , shadowBias(0.001f)
    , shadowStrength(0.8f)
    , lightSpaceMatrix(1.0f)
    , currentLight(nullptr)
    , currentCamera(nullptr) {
}

ShadowManager::~ShadowManager() {
    cleanup();
}

void ShadowManager::initialize(int size) {
    if (initialized) cleanup();
    
    shadowMapSize = size;
    std::cout << "ShadowManager: Initializing simple shadows (size: " << shadowMapSize << ")" << std::endl;
    
    createFramebuffer();
    createShadowShader();
    
    initialized = true;
    std::cout << "ShadowManager: Simple shadows initialized successfully" << std::endl;
}

void ShadowManager::cleanup() {
    if (!initialized) return;
    
    if (depthTexture) {
        glDeleteTextures(1, &depthTexture);
        depthTexture = 0;
    }
    
    if (framebuffer) {
        glDeleteFramebuffers(1, &framebuffer);
        framebuffer = 0;
    }
    
    if (shadowShader) {
        glDeleteProgram(shadowShader);
        shadowShader = 0;
    }
    
    initialized = false;
    std::cout << "ShadowManager: Cleaned up" << std::endl;
}

void ShadowManager::createFramebuffer() {
    // Generate framebuffer
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    
    // Generate depth texture
    glGenTextures(1, &depthTexture);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, shadowMapSize, shadowMapSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    
    // Set texture parameters for shadow mapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    
    // Set border color to white (no shadow)
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    
    // CORREGIDO: Configuración optimizada para shadow comparison
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    
    // Attach depth texture as FBO's depth buffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
    
    // No color buffer needed
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    
    // Check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "ShadowManager: ERROR - Framebuffer not complete!" << std::endl;
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    std::cout << "ShadowManager: Framebuffer created (FB: " << framebuffer << ", Texture: " << depthTexture << ")" << std::endl;
}

void ShadowManager::createShadowShader() {
    const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
// CORREGIDO: Usar locations 2-5 para instanced rendering como en tu AssimpGeometry
layout (location = 2) in vec4 aInstanceMatrix_0;
layout (location = 3) in vec4 aInstanceMatrix_1;
layout (location = 4) in vec4 aInstanceMatrix_2;
layout (location = 5) in vec4 aInstanceMatrix_3;

uniform mat4 uLightSpaceMatrix;

void main() {
    // Reconstruir la matriz de instancia desde los 4 vec4
    mat4 aInstanceMatrix = mat4(aInstanceMatrix_0, aInstanceMatrix_1, aInstanceMatrix_2, aInstanceMatrix_3);
    gl_Position = uLightSpaceMatrix * aInstanceMatrix * vec4(aPos, 1.0);
}
)";

    const char* fragmentShaderSource = R"(
#version 330 core

void main() {
    // gl_FragDepth is automatically written
}
)";
    
    // Create vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    
    // Check compilation
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cout << "ShadowManager: Vertex shader compilation failed: " << infoLog << std::endl;
    }
    
    // Create fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cout << "ShadowManager: Fragment shader compilation failed: " << infoLog << std::endl;
    }
    
    // Create shader program
    shadowShader = glCreateProgram();
    glAttachShader(shadowShader, vertexShader);
    glAttachShader(shadowShader, fragmentShader);
    glLinkProgram(shadowShader);
    
    glGetProgramiv(shadowShader, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shadowShader, 512, nullptr, infoLog);
        std::cout << "ShadowManager: Shader program linking failed: " << infoLog << std::endl;
    }
    
    // Clean up shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    std::cout << "ShadowManager: Shadow shader created (ID: " << shadowShader << ")" << std::endl;
}

void ShadowManager::beginShadowPass(std::shared_ptr<Light> directionalLight, Camera* camera) {
    if (!initialized || !directionalLight || !camera) {
        std::cout << "ShadowManager: Cannot begin shadow pass - not initialized or missing parameters" << std::endl;
        return;
    }
    
    currentLight = directionalLight;
    currentCamera = camera;
    
    // Calculate light space matrix
    lightSpaceMatrix = calculateLightSpaceMatrix(directionalLight, camera);
    
    // Bind shadow framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
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
    
    // CORREGIDO: Asegurar configuración correcta para shadow mapping
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glCullFace(GL_FRONT); // Peter panning fix: renderizar back faces en shadow map
    glEnable(GL_CULL_FACE);
    
    std::cout << "ShadowManager: Shadow pass started (FB: " << framebuffer << ", Shader: " << shadowShader 
              << ", Resolution: " << shadowMapSize << "x" << shadowMapSize << ")" << std::endl;
}

void ShadowManager::endShadowPass() {
    // Restore default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // CORREGIDO: Restaurar estado de OpenGL
    glCullFace(GL_BACK); // Restaurar culling normal
    glDepthFunc(GL_LESS); // Asegurar depth func correcto
    
    // Reset viewport (will be set by render pipeline)
    // Reset shader program (will be set by render pipeline)
    
    std::cout << "ShadowManager: Shadow pass ended" << std::endl;
}

void ShadowManager::bindShadowMap(GLuint shaderProgram) {
    if (!initialized) return;
    
    // Bind shadow map to texture unit 10
    glActiveTexture(GL_TEXTURE10);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    
    // Set shadow map uniform
    GLint shadowMapLoc = glGetUniformLocation(shaderProgram, "uShadowMap");
    glUniform1i(shadowMapLoc, 10);
    
    std::cout << "ShadowManager: Shadow map bound to texture unit 10 (location: " << shadowMapLoc << ")" << std::endl;
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
    
    std::cout << "ShadowManager: Shadow uniforms set - Matrix(" << lightSpaceMatrixLoc 
              << "), Bias: " << shadowBias << "(" << shadowBiasLoc 
              << "), Strength: " << shadowStrength << "(" << shadowStrengthLoc 
              << "), Enabled(" << enableShadowsLoc << ")" << std::endl;
}

glm::mat4 ShadowManager::calculateLightSpaceMatrix(std::shared_ptr<Light> light, Camera* camera) {
    // Get light direction - asegurarse de que est� normalizada
    glm::vec3 lightDir = glm::normalize(light->getDirection());
    std::cout << "ShadowManager: Light direction: (" << lightDir.x << ", " << lightDir.y << ", " << lightDir.z << ")" << std::endl;

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
            // Recreate framebuffer with new size
            if (depthTexture) glDeleteTextures(1, &depthTexture);
            if (framebuffer) glDeleteFramebuffers(1, &framebuffer);
            createFramebuffer();
        }
    }
}