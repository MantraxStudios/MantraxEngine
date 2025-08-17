#pragma once
#include <glm/glm.hpp>
#include "../core/CoreExporter.h"

enum class MANTRAXCORE_API AspectRatioMode {
    Stretch,        // Estira la UI para llenar toda la pantalla (puede deformar)
    Letterbox,      // Mantiene aspecto ratio agregando barras negras
    Crop,           // Mantiene aspecto ratio pero corta partes de la imagen
    Adaptive        // Se adapta dinámicamente al tamaño de pantalla
};

struct MANTRAXCORE_API AspectRatioSettings {
    float targetAspectRatio = 16.0f / 9.0f;  // Aspecto ratio objetivo (1920x1080)
    AspectRatioMode mode = AspectRatioMode::Adaptive;
    bool maintainPixelPerfect = false;
    float uiScale = 1.0f;
};

class MANTRAXCORE_API AspectRatioManager {
public:
    static AspectRatioManager& getInstance();
    
    // Configuración del aspect ratio
    void setSettings(const AspectRatioSettings& settings);
    AspectRatioSettings getSettings() const { return settings; }
    
    // Cálculos de viewport y escalado
    glm::ivec4 calculateViewport(int screenWidth, int screenHeight) const;
    glm::vec2 calculateUIScale(int screenWidth, int screenHeight) const;
    glm::mat4 calculateUIProjectionMatrix(int uiWidth, int uiHeight) const;
    
    // Conversión de coordenadas
    glm::vec2 screenToUI(const glm::vec2& screenPos, int screenWidth, int screenHeight, int uiWidth, int uiHeight) const;
    glm::vec2 uiToScreen(const glm::vec2& uiPos, int screenWidth, int screenHeight, int uiWidth, int uiHeight) const;
    
    // Información del estado actual
    float getCurrentAspectRatio() const { return currentAspectRatio; }
    glm::vec2 getCurrentUIScale() const { return currentUIScale; }
    glm::ivec4 getCurrentViewport() const { return currentViewport; }
    
    // Actualización cuando cambia el tamaño de pantalla
    void updateScreenSize(int width, int height);
    
    // Configuraciones predefinidas
    static AspectRatioSettings createLetterboxSettings(float targetAspect = 16.0f / 9.0f);
    static AspectRatioSettings createAdaptiveSettings();
    static AspectRatioSettings createPixelPerfectSettings();

private:
    AspectRatioManager() = default;
    ~AspectRatioManager() = default;
    AspectRatioManager(const AspectRatioManager&) = delete;
    AspectRatioManager& operator=(const AspectRatioManager&) = delete;
    
    AspectRatioSettings settings;
    float currentAspectRatio = 16.0f / 9.0f;
    glm::vec2 currentUIScale = glm::vec2(1.0f);
    glm::ivec4 currentViewport = glm::ivec4(0, 0, 1920, 1080);
    
    glm::ivec4 calculateLetterboxViewport(int screenWidth, int screenHeight) const;
    glm::ivec4 calculateCropViewport(int screenWidth, int screenHeight) const;
    glm::ivec4 calculateAdaptiveViewport(int screenWidth, int screenHeight) const;
};
