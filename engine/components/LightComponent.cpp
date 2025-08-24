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
            std::cout << "LightComponent: Light added to scene for GameObject '" << owner->Name << "'" << std::endl;
        }
        
        // CORREGIDO: Actualizar transformación inmediatamente
        updateTransform();
        
        // Mostrar información inicial de la luz
        std::cout << "LightComponent: Initial light configuration:" << std::endl;
        std::cout << "  Type: " << (light->getType() == LightType::Directional ? "Directional" : 
                                   light->getType() == LightType::Spot ? "Spot" : "Point") << std::endl;
        std::cout << "  Color: (" << light->getColor().x << ", " << light->getColor().y << ", " << light->getColor().z << ")" << std::endl;
        std::cout << "  Intensity: " << light->getIntensity() << std::endl;
        std::cout << "  Position: (" << light->getPosition().x << ", " << light->getPosition().y << ", " << light->getPosition().z << ")" << std::endl;
        std::cout << "  Direction: (" << light->getDirection().x << ", " << light->getDirection().y << ", " << light->getDirection().z << ")" << std::endl;
    }
}

void LightComponent::update() {
    if (owner && light) {
        // CORREGIDO: Sincronizar todas las propiedades del componente con la luz
        light->setColor(color);
        light->setIntensity(intensity);
        light->setEnabled(enabled);

        // Sincronizar propiedades específicas por tipo
        switch (light->getType()) {
            case LightType::Point:
                light->setAttenuation(attenuationConstant, attenuationLinear, attenuationQuadratic);
                light->setRange(minDistance, maxDistance);
                break;
            case LightType::Spot:
                light->setCutOffAngle(cutOffAngle);
                light->setOuterCutOffAngle(outerCutOffAngle);
                light->setSpotRange(spotMaxDistance);
                light->setAttenuation(attenuationConstant, attenuationLinear, attenuationQuadratic);
                light->setRange(minDistance, maxDistance);
                break;
            case LightType::Directional:
                // Las luces direccionales no necesitan atenuación ni rango
                break;
        }

        // Actualizar transformación (posición y dirección)
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

    // CORREGIDO: Calcular dirección basada en la rotación mundial
    // Usar el vector forward estándar (0, 0, -1) y aplicarle la rotación
    glm::vec3 forward = worldRot * glm::vec3(0.0f, 0.0f, -1.0f);
    
    // Para luces direccionales, la dirección es opuesta al forward del objeto
    if (light->getType() == LightType::Directional) {
        forward = -forward; // Invertir para que apunte en la dirección correcta
    }
    
    // Normalizar y establecer la dirección
    glm::vec3 normalizedDirection = glm::normalize(forward);
    light->setDirection(normalizedDirection);
    
    // DEBUG: Mostrar información de la transformación
    std::cout << "LightComponent: GameObject '" << owner->Name << "' transform updated:" << std::endl;
    std::cout << "  Position: (" << worldPos.x << ", " << worldPos.y << ", " << worldPos.z << ")" << std::endl;
    std::cout << "  Rotation: (" << glm::degrees(glm::eulerAngles(worldRot).x) << "°, " 
              << glm::degrees(glm::eulerAngles(worldRot).y) << "°, " 
              << glm::degrees(glm::eulerAngles(worldRot).z) << "°)" << std::endl;
    std::cout << "  Forward: (" << forward.x << ", " << forward.y << ", " << forward.z << ")" << std::endl;
    std::cout << "  Light Direction: (" << normalizedDirection.x << ", " << normalizedDirection.y << ", " << normalizedDirection.z << ")" << std::endl;
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

// CORREGIDO: Método para establecer dirección manualmente
void LightComponent::setDirection(const glm::vec3& direction) {
    if (light) {
        light->setDirection(direction);
        std::cout << "LightComponent: Direction set manually to (" 
                  << direction.x << ", " << direction.y << ", " << direction.z << ")" << std::endl;
    }
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

// CORREGIDO: Obtener dirección actual de la luz
glm::vec3 LightComponent::getDirection() const {
    return light ? light->getDirection() : glm::vec3(0.0f, -1.0f, 0.0f);
}

// CORREGIDO: Obtener posición actual de la luz
glm::vec3 LightComponent::getPosition() const {
    return light ? light->getPosition() : glm::vec3(0.0f);
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

// CORREGIDO: Método para verificar el estado de la luz
void LightComponent::debugLightStatus() const {
    if (!light) {
        std::cout << "LightComponent: ERROR - No hay luz asociada" << std::endl;
        return;
    }
    
    if (!owner) {
        std::cout << "LightComponent: ERROR - No hay GameObject asociado" << std::endl;
        return;
    }
    
    std::cout << "=== LIGHT DEBUG STATUS ===" << std::endl;
    std::cout << "GameObject: '" << owner->Name << "'" << std::endl;
    
    // Información del GameObject
    glm::vec3 worldPos = owner->getWorldPosition();
    glm::quat worldRot = owner->getWorldRotationQuat();
    glm::vec3 localPos = owner->getLocalPosition();
    glm::quat localRot = owner->getLocalRotationQuat();
    
    std::cout << "GameObject Transform:" << std::endl;
    std::cout << "  Local Position: (" << localPos.x << ", " << localPos.y << ", " << localPos.z << ")" << std::endl;
    std::cout << "  Local Rotation: (" << localRot.x << ", " << localRot.y << ", " << localRot.z << ", " << localRot.w << ")" << std::endl;
    std::cout << "  World Position: (" << worldPos.x << ", " << worldPos.y << ", " << worldPos.z << ")" << std::endl;
    std::cout << "  World Rotation: (" << worldRot.x << ", " << worldRot.y << ", " << worldRot.z << ", " << worldRot.w << ")" << std::endl;
    
    // Información de la luz
    std::cout << "Light Properties:" << std::endl;
    std::cout << "  Type: " << (light->getType() == LightType::Directional ? "Directional" : 
                               light->getType() == LightType::Spot ? "Spot" : "Point") << std::endl;
    std::cout << "  Color: (" << light->getColor().x << ", " << light->getColor().y << ", " << light->getColor().z << ")" << std::endl;
    std::cout << "  Intensity: " << light->getIntensity() << std::endl;
    std::cout << "  Position: (" << light->getPosition().x << ", " << light->getPosition().y << ", " << light->getPosition().z << ")" << std::endl;
    std::cout << "  Direction: (" << light->getDirection().x << ", " << light->getDirection().y << ", " << light->getDirection().z << ")" << std::endl;
    std::cout << "  Enabled: " << (light->isEnabled() ? "Yes" : "No") << std::endl;
    
    // Propiedades específicas por tipo
    switch (light->getType()) {
        case LightType::Spot:
            std::cout << "  CutOff Angle: " << glm::degrees(light->getCutOffAngle()) << "°" << std::endl;
            std::cout << "  Outer CutOff Angle: " << glm::degrees(light->getOuterCutOffAngle()) << "°" << std::endl;
            std::cout << "  Spot Range: " << light->getSpotRange() << std::endl;
            break;
        case LightType::Point:
            glm::vec3 atten = light->getAttenuation();
            std::cout << "  Attenuation: (" << atten.x << ", " << atten.y << ", " << atten.z << ")" << std::endl;
            std::cout << "  Range: " << light->getMinDistance() << " to " << light->getMaxDistance() << std::endl;
            break;
        case LightType::Directional:
            std::cout << "  Directional lights don't have range or attenuation" << std::endl;
            break;
    }
    
    // Verificar sincronización
    bool posSynced = (worldPos == light->getPosition());
    std::cout << "Synchronization:" << std::endl;
    std::cout << "  Position Synced: " << (posSynced ? "Yes" : "No") << std::endl;
    
    if (!posSynced) {
        std::cout << "  WARNING: GameObject and light positions are not synchronized!" << std::endl;
    }
    
    std::cout << "=========================" << std::endl;
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
