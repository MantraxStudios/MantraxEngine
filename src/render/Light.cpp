#include "Light.h"
#include <glm/gtc/constants.hpp>

Light::Light(LightType lightType)
    : type(lightType), color(1.0f), intensity(1.0f), position(0.0f), 
      direction(0.0f, -1.0f, 0.0f), cutOffAngle(glm::radians(12.5f)), 
      outerCutOffAngle(glm::radians(17.5f)), spotRange(50.0f),
      attenuation(1.0f, 0.09f, 0.032f), minDistance(0.1f), maxDistance(50.0f),
      enabled(true) {
}

void Light::setColor(const glm::vec3& lightColor) {
    color = lightColor;
}

void Light::setIntensity(float lightIntensity) {
    intensity = lightIntensity;
}

void Light::setPosition(const glm::vec3& lightPosition) {
    position = lightPosition;
}

void Light::setDirection(const glm::vec3& lightDirection) {
    direction = glm::normalize(lightDirection);
}

void Light::setCutOffAngle(float angle) {
    // Convertir de grados a radianes y asegurar que esté en un rango válido
    angle = glm::clamp(angle, 0.0f, 89.0f);
    cutOffAngle = glm::radians(angle);
    
    // Asegurar que el ángulo exterior sea siempre mayor
    float outerAngleDegrees = glm::degrees(outerCutOffAngle);
    if (angle >= outerAngleDegrees) {
        outerCutOffAngle = glm::radians(angle + 5.0f);
    }
}

void Light::setOuterCutOffAngle(float angle) {
    // Convertir de grados a radianes y asegurar que esté en un rango válido
    angle = glm::clamp(angle, 0.0f, 90.0f);
    float innerAngleDegrees = glm::degrees(cutOffAngle);
    
    // Asegurar que el ángulo exterior sea siempre mayor que el interior
    if (angle <= innerAngleDegrees) {
        angle = innerAngleDegrees + 5.0f;
    }
    
    outerCutOffAngle = glm::radians(angle);
}

void Light::setSpotRange(float maxDistance) {
    spotRange = glm::max(0.1f, maxDistance);
}

void Light::setAttenuation(float constant, float linear, float quadratic) {
    attenuation = glm::vec3(constant, linear, quadratic);
}

void Light::setRange(float min, float max) {
    minDistance = glm::max(0.1f, min); // Evitar valores demasiado pequeños
    maxDistance = glm::max(minDistance + 0.1f, max); // Asegurar que max > min
} 