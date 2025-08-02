#include "LightComponent.h"
#include "GameObject.h"
#include "SceneManager.h"
#include <iostream>

using json = nlohmann::json;

void LightComponent::defines() {
    set_var("Color", &color);                     // glm::vec3*
    set_var("Intensity", &intensity);             // float*
    set_var("Enabled", &enabled);                 // bool*

    set_var("Attenuation Constant", &attenuationConstant);   // float*
    set_var("Attenuation Linear", &attenuationLinear);       // float*
    set_var("Attenuation Quadratic", &attenuationQuadratic); // float*

    set_var("Min Distance", &minDistance);         // float*
    set_var("Max Distance", &maxDistance);         // float*

    set_var("CutOff Angle", &cutOffAngle);         // float*
    set_var("Outer CutOff Angle", &outerCutOffAngle); // float*

    set_var("Spot Max Distance", &spotMaxDistance); // float*
}

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
        //setColor(color);
        //setIntensity(intensity);
        //setEnabled(enabled);

        //setAttenuation(attenuationConstant, attenuationLinear, attenuationQuadratic);
        //setRange(minDistance, maxDistance);

        //setCutOffAngle(cutOffAngle);
        //setOuterCutOffAngle(outerCutOffAngle);

        //setSpotRange(spotMaxDistance);

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

std::string LightComponent::serializeComponent() const {
    json j;
    if (!light) return "{ }";

    j["type"] = static_cast<int>(light->getType());

    glm::vec3 color = light->getColor();
    j["color"] = { color.x, color.y, color.z };

    j["intensity"] = light->getIntensity();
    j["enabled"] = light->isEnabled();

    glm::vec3 attenuation = light->getAttenuation();
    j["attenuation"] = { attenuation.x, attenuation.y, attenuation.z };

    j["minDistance"] = light->getMinDistance();
    j["maxDistance"] = light->getMaxDistance();

    if (light->getType() == LightType::Spot) {
        j["cutOffAngle"] = light->getCutOffAngle();
        j["outerCutOffAngle"] = light->getOuterCutOffAngle();
        j["spotRange"] = light->getSpotRange();
    }
    
    return j.dump();
}

void LightComponent::deserialize(const std::string& data) {
    json j = json::parse(data);

    LightType type = static_cast<LightType>(j.value("type", static_cast<int>(LightType::Point)));
    if (!light || light->getType() != type) {
        light = std::make_shared<Light>(type);
    }

    if (j.contains("color") && j["color"].is_array()) {
        auto c = j["color"];
        light->setColor(glm::vec3(c[0], c[1], c[2]));
    }
    light->setIntensity(j.value("intensity", 1.0f));
    light->setEnabled(j.value("enabled", true));

    if (j.contains("attenuation") && j["attenuation"].is_array()) {
        auto a = j["attenuation"];
        light->setAttenuation(a[0], a[1], a[2]);
    }
    light->setRange(j.value("minDistance", 0.1f), j.value("maxDistance", 50.0f));

    if (type == LightType::Spot) {
        light->setCutOffAngle(j.value("cutOffAngle", 12.5f));
        light->setOuterCutOffAngle(j.value("outerCutOffAngle", 17.5f));
        light->setSpotRange(j.value("spotRange", 50.0f));
    }
}
