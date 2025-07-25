#include "Light.h"
#include <glm/gtc/constants.hpp>

Light::Light(LightType lightType)
    : type(lightType), color(1.0f), intensity(1.0f), position(0.0f), 
      direction(0.0f, -1.0f, 0.0f), cutOffAngle(glm::radians(12.5f)), 
      outerCutOffAngle(glm::radians(17.5f)), attenuation(1.0f, 0.09f, 0.032f), 
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
    cutOffAngle = glm::radians(angle);
}

void Light::setOuterCutOffAngle(float angle) {
    outerCutOffAngle = glm::radians(angle);
}

void Light::setAttenuation(float constant, float linear, float quadratic) {
    attenuation = glm::vec3(constant, linear, quadratic);
} 