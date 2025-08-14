#include "AspectRatioManager.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <iostream>

AspectRatioManager& AspectRatioManager::getInstance() {
    static AspectRatioManager instance;
    return instance;
}

void AspectRatioManager::setSettings(const AspectRatioSettings& newSettings) {
    settings = newSettings;
    std::cout << "[AspectRatioManager] Settings updated: mode=" << static_cast<int>(settings.mode) 
              << ", targetAspect=" << settings.targetAspectRatio 
              << ", uiScale=" << settings.uiScale << std::endl;
}

void AspectRatioManager::updateScreenSize(int width, int height) {
    if (width <= 0 || height <= 0) {
        std::cerr << "[AspectRatioManager] Invalid screen size: " << width << "x" << height << std::endl;
        return;
    }
    
    currentAspectRatio = static_cast<float>(width) / height;
    currentViewport = calculateViewport(width, height);
    currentUIScale = calculateUIScale(width, height);
    
    std::cout << "[AspectRatioManager] Screen size updated: " << width << "x" << height 
              << ", aspect=" << currentAspectRatio 
              << ", viewport=(" << currentViewport.x << "," << currentViewport.y 
              << "," << currentViewport.z << "," << currentViewport.w << ")" 
              << ", uiScale=(" << currentUIScale.x << "," << currentUIScale.y << ")" << std::endl;
}

glm::ivec4 AspectRatioManager::calculateViewport(int screenWidth, int screenHeight) const {
    switch (settings.mode) {
        case AspectRatioMode::Letterbox:
            return calculateLetterboxViewport(screenWidth, screenHeight);
        case AspectRatioMode::Crop:
            return calculateCropViewport(screenWidth, screenHeight);
        case AspectRatioMode::Adaptive:
            return calculateAdaptiveViewport(screenWidth, screenHeight);
        case AspectRatioMode::Stretch:
        default:
            return glm::ivec4(0, 0, screenWidth, screenHeight);
    }
}

glm::ivec4 AspectRatioManager::calculateLetterboxViewport(int screenWidth, int screenHeight) const {
    float screenAspect = static_cast<float>(screenWidth) / screenHeight;
    
    if (screenAspect > settings.targetAspectRatio) {
        // Pantalla más ancha que el objetivo, agregar barras a los lados
        int viewportWidth = static_cast<int>(screenHeight * settings.targetAspectRatio);
        int viewportX = (screenWidth - viewportWidth) / 2;
        return glm::ivec4(viewportX, 0, viewportWidth, screenHeight);
    } else {
        // Pantalla más alta que el objetivo, agregar barras arriba y abajo
        int viewportHeight = static_cast<int>(screenWidth / settings.targetAspectRatio);
        int viewportY = (screenHeight - viewportHeight) / 2;
        return glm::ivec4(0, viewportY, screenWidth, viewportHeight);
    }
}

glm::ivec4 AspectRatioManager::calculateCropViewport(int screenWidth, int screenHeight) const {
    float screenAspect = static_cast<float>(screenWidth) / screenHeight;
    
    if (screenAspect < settings.targetAspectRatio) {
        // Pantalla más alta, recortar arriba y abajo
        int viewportHeight = static_cast<int>(screenWidth / settings.targetAspectRatio);
        int viewportY = (screenHeight - viewportHeight) / 2;
        return glm::ivec4(0, viewportY, screenWidth, viewportHeight);
    } else {
        // Pantalla más ancha, recortar a los lados
        int viewportWidth = static_cast<int>(screenHeight * settings.targetAspectRatio);
        int viewportX = (screenWidth - viewportWidth) / 2;
        return glm::ivec4(viewportX, 0, viewportWidth, screenHeight);
    }
}

glm::ivec4 AspectRatioManager::calculateAdaptiveViewport(int screenWidth, int screenHeight) const {
    // En modo adaptativo, usar toda la pantalla pero escalar la UI apropiadamente
    return glm::ivec4(0, 0, screenWidth, screenHeight);
}

glm::vec2 AspectRatioManager::calculateUIScale(int screenWidth, int screenHeight) const {
    float baseUIScale = settings.uiScale;
    
    switch (settings.mode) {
        case AspectRatioMode::Adaptive: {
            // Escalar basado en la resolución (usando 1920x1080 como base)
            float scaleX = static_cast<float>(screenWidth) / 1920.0f;
            float scaleY = static_cast<float>(screenHeight) / 1080.0f;
            
            if (settings.maintainPixelPerfect) {
                // Usar el menor de los dos para mantener pixel perfect
                float uniformScale = std::min(scaleX, scaleY);
                return glm::vec2(uniformScale * baseUIScale);
            } else {
                // Escalar independientemente en cada eje
                return glm::vec2(scaleX * baseUIScale, scaleY * baseUIScale);
            }
        }
        case AspectRatioMode::Letterbox:
        case AspectRatioMode::Crop: {
            // Para letterbox y crop, calcular escala basada en el viewport efectivo
            glm::ivec4 viewport = calculateViewport(screenWidth, screenHeight);
            float scaleX = static_cast<float>(viewport.z) / 1920.0f;
            float scaleY = static_cast<float>(viewport.w) / 1080.0f;
            float uniformScale = std::min(scaleX, scaleY);
            return glm::vec2(uniformScale * baseUIScale);
        }
        case AspectRatioMode::Stretch:
        default: {
            // En modo stretch, escalar para llenar completamente
            float scaleX = static_cast<float>(screenWidth) / 1920.0f;
            float scaleY = static_cast<float>(screenHeight) / 1080.0f;
            return glm::vec2(scaleX * baseUIScale, scaleY * baseUIScale);
        }
    }
}

glm::mat4 AspectRatioManager::calculateUIProjectionMatrix(int uiWidth, int uiHeight) const {
    // Crear matriz ortográfica para UI (Y invertida para que 0,0 esté en la esquina superior izquierda)
    return glm::ortho(0.0f, static_cast<float>(uiWidth), static_cast<float>(uiHeight), 0.0f, -1.0f, 1.0f);
}

glm::vec2 AspectRatioManager::screenToUI(const glm::vec2& screenPos, int screenWidth, int screenHeight, int uiWidth, int uiHeight) const {
    glm::ivec4 viewport = calculateViewport(screenWidth, screenHeight);
    
    // Convertir posición de pantalla a posición de viewport
    glm::vec2 viewportPos = screenPos - glm::vec2(viewport.x, viewport.y);
    
    // Escalar a coordenadas UI
    glm::vec2 uiPos;
    uiPos.x = (viewportPos.x / viewport.z) * uiWidth;
    uiPos.y = (viewportPos.y / viewport.w) * uiHeight;
    
    return uiPos;
}

glm::vec2 AspectRatioManager::uiToScreen(const glm::vec2& uiPos, int screenWidth, int screenHeight, int uiWidth, int uiHeight) const {
    glm::ivec4 viewport = calculateViewport(screenWidth, screenHeight);
    
    // Convertir coordenadas UI a viewport
    glm::vec2 viewportPos;
    viewportPos.x = (uiPos.x / uiWidth) * viewport.z;
    viewportPos.y = (uiPos.y / uiHeight) * viewport.w;
    
    // Convertir a coordenadas de pantalla
    return viewportPos + glm::vec2(viewport.x, viewport.y);
}

// Configuraciones predefinidas
AspectRatioSettings AspectRatioManager::createLetterboxSettings(float targetAspect) {
    AspectRatioSettings settings;
    settings.mode = AspectRatioMode::Letterbox;
    settings.targetAspectRatio = targetAspect;
    settings.maintainPixelPerfect = true;
    settings.uiScale = 1.0f;
    return settings;
}

AspectRatioSettings AspectRatioManager::createAdaptiveSettings() {
    AspectRatioSettings settings;
    settings.mode = AspectRatioMode::Adaptive;
    settings.targetAspectRatio = 16.0f / 9.0f;
    settings.maintainPixelPerfect = false;
    settings.uiScale = 1.0f;
    return settings;
}

AspectRatioSettings AspectRatioManager::createPixelPerfectSettings() {
    AspectRatioSettings settings;
    settings.mode = AspectRatioMode::Adaptive;
    settings.targetAspectRatio = 16.0f / 9.0f;
    settings.maintainPixelPerfect = true;
    settings.uiScale = 1.0f;
    return settings;
}
