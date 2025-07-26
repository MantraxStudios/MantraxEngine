#include "LightComponent.h"
#include "GameObject.h"
#include "SceneManager.h"
#include <iostream>

LightComponent::LightComponent(LightType type) {
    light = std::make_shared<Light>(type);
    
    // Configurar valores por defecto según el tipo
    switch (type) {
        case LightType::Point:
            light->setAttenuation(1.0f, 0.09f, 0.032f);
            light->setRange(0.1f, 50.0f);
            break;
        case LightType::Spot:
            light->setCutOffAngle(12.5f);
            light->setOuterCutOffAngle(17.5f);
            light->setSpotRange(50.0f);
            break;
        case LightType::Directional:
            break;
    }

    // Valores comunes por defecto
    light->setColor(glm::vec3(1.0f));
    light->setIntensity(1.0f);
}

void LightComponent::setOwner(GameObject* owner) {
    // Si ya teníamos un owner, remover la luz de la escena actual
    if (this->owner && light) {
        if (auto scene = SceneManager::getInstance().getActiveScene()) {
            scene->removeLight(light);
        }
    }

    Component::setOwner(owner);

    // Si tenemos nuevo owner, agregar la luz a la escena
    if (owner && light) {
        if (auto scene = SceneManager::getInstance().getActiveScene()) {
            scene->addLight(light);
            std::cout << "Light added to scene at position: " 
                      << owner->getWorldPosition().x << ", "
                      << owner->getWorldPosition().y << ", "
                      << owner->getWorldPosition().z << std::endl;
        }
        updateTransform();
    }
}

void LightComponent::update() {
    if (owner && light) {
        updateTransform();
    }
}

void LightComponent::updateTransform() {
    if (!owner || !light) return;

    // Obtener la transformación mundial del objeto
    glm::vec3 worldPos = owner->getWorldPosition();
    glm::quat worldRot = owner->getWorldRotationQuat();

    // Actualizar posición
    light->setPosition(worldPos);

    // Calcular dirección basada en la rotación mundial
    glm::vec3 forward = worldRot * glm::vec3(0.0f, 0.0f, -1.0f);
    light->setDirection(glm::normalize(forward));
}

// Delegación de funciones
void LightComponent::setColor(const glm::vec3& color) {
    if (light) light->setColor(color);
}

void LightComponent::setIntensity(float intensity) {
    if (light) light->setIntensity(intensity);
}

void LightComponent::setEnabled(bool enable) {
    if (light) light->setEnabled(enable);
}

void LightComponent::setAttenuation(float constant, float linear, float quadratic) {
    if (light) light->setAttenuation(constant, linear, quadratic);
}

void LightComponent::setRange(float minDistance, float maxDistance) {
    if (light) light->setRange(minDistance, maxDistance);
}

void LightComponent::setCutOffAngle(float angle) {
    if (light) light->setCutOffAngle(angle);
}

void LightComponent::setOuterCutOffAngle(float angle) {
    if (light) light->setOuterCutOffAngle(angle);
}

void LightComponent::setSpotRange(float maxDistance) {
    if (light) light->setSpotRange(maxDistance);
}

// Getters
LightType LightComponent::getType() const {
    return light ? light->getType() : LightType::Point;
}

glm::vec3 LightComponent::getColor() const {
    return light ? light->getColor() : glm::vec3(1.0f);
}

float LightComponent::getIntensity() const {
    return light ? light->getIntensity() : 1.0f;
}

bool LightComponent::isEnabled() const {
    return light ? light->isEnabled() : false;
}

float LightComponent::getCutOffAngle() const {
    return light ? light->getCutOffAngle() : 0.0f;
}

float LightComponent::getOuterCutOffAngle() const {
    return light ? light->getOuterCutOffAngle() : 0.0f;
}

float LightComponent::getSpotRange() const {
    return light ? light->getSpotRange() : 0.0f;
}

glm::vec3 LightComponent::getAttenuation() const {
    return light ? light->getAttenuation() : glm::vec3(1.0f, 0.0f, 0.0f);
}

float LightComponent::getMinDistance() const {
    return light ? light->getMinDistance() : 0.1f;
}

float LightComponent::getMaxDistance() const {
    return light ? light->getMaxDistance() : 50.0f;
} 