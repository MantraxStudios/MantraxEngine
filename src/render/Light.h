#pragma once
#include <glm/glm.hpp>
#include "../core/CoreExporter.h"


enum class MANTRAXCORE_API LightType {
    Directional,
    Point,
    Spot
};

class MANTRAXCORE_API Light {
public:
    Light(LightType type);
    virtual ~Light() = default;

    // Propiedades básicas
    void setColor(const glm::vec3& color);
    void setIntensity(float intensity);
    void setPosition(const glm::vec3& position);
    void setDirection(const glm::vec3& direction);

    // Propiedades específicas para Spot Light
    void setCutOffAngle(float angle);
    void setOuterCutOffAngle(float angle);

    // Propiedades específicas para Point Light
    void setAttenuation(float constant, float linear, float quadratic);

    // Getters
    LightType getType() const { return type; }
    glm::vec3 getColor() const { return color; }
    float getIntensity() const { return intensity; }
    glm::vec3 getPosition() const { return position; }
    glm::vec3 getDirection() const { return direction; }
    float getCutOffAngle() const { return cutOffAngle; }
    float getOuterCutOffAngle() const { return outerCutOffAngle; }
    glm::vec3 getAttenuation() const { return attenuation; }

    // Métodos de utilidad
    bool isEnabled() const { return enabled; }
    void setEnabled(bool enable) { enabled = enable; }

protected:
    LightType type;
    glm::vec3 color;
    float intensity;
    glm::vec3 position;
    glm::vec3 direction;
    
    // Spot light properties
    float cutOffAngle;
    float outerCutOffAngle;
    
    // Point light properties
    glm::vec3 attenuation; // constant, linear, quadratic
    
    bool enabled;
}; 