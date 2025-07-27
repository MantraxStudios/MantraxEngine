#pragma once
#include "Component.h"
#include "../render/Light.h"
#include "../core/CoreExporter.h"
#include <memory>

class MANTRAXCORE_API LightComponent : public Component {
public:
    LightComponent(LightType type = LightType::Point);
    void setOwner(GameObject* owner) override;
    void update() override;

    // Validación del componente
    bool isValid() const override { return Component::isValid() && light != nullptr; }

    // Delegación de funciones de Light
    void setColor(const glm::vec3& color);
    void setIntensity(float intensity);
    void setEnabled(bool enable);
    
    // Funciones específicas por tipo de luz
    void setAttenuation(float constant, float linear, float quadratic);
    void setRange(float minDistance, float maxDistance);
    void setCutOffAngle(float angle);
    void setOuterCutOffAngle(float angle);
    void setSpotRange(float maxDistance);

    // Getters
    LightType getType() const;
    glm::vec3 getColor() const;
    float getIntensity() const;
    bool isEnabled() const;
    float getCutOffAngle() const;
    float getOuterCutOffAngle() const;
    float getSpotRange() const;
    glm::vec3 getAttenuation() const;
    float getMinDistance() const;
    float getMaxDistance() const;
    std::shared_ptr<Light> getLight() const { return light; }

private:
    std::shared_ptr<Light> light;
    void updateTransform();
}; 