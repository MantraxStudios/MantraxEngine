#pragma once
#include "Component.h"
#include <fmod/fmod.hpp>
#include <iostream>
#include "../core/CoreExporter.h"

class MANTRAXCORE_API AudioSource : public Component {
public:
    AudioSource() = default;
    void setOwner(GameObject* owner) override;

    // Validación del componente
    bool isValid() const override { return Component::isValid() && !soundPath.empty(); }

    // Configuración del sonido
    void setSound(const std::string& path, bool is3D = true, bool isLooping = false, bool isStream = false);
    void setVolume(float volume);
    void set3DAttributes(bool enabled);
    void setMinDistance(float distance);
    void setMaxDistance(float distance);


    // Control de reproducción
    void play();
    void stop();
    void pause();
    void resume();

    // Estado
    bool isPlaying() const;
    bool isPaused() const;
    float getVolume() const { return volume; }
    bool is3DEnabled() const { return is3D; }
    float getMinDistance() const { return minDistance; }
    float getMaxDistance() const { return maxDistance; }

    // Actualización del componente
    void update() override;
    void destroy() override;
    void deserialize(const std::string& data) override;
    std::string serializeComponent() const override;

private:
    std::string soundPath;
    FMOD::Channel* channel = nullptr;
    float volume = 1.0f;
    bool is3D = true;
    bool isLooping = false;
    bool isStream = false;
    bool shouldPlay = false;
    float minDistance = 5.0f;     // Distancia mínima para atenuación (ajustada a 5 unidades)
    float maxDistance = 100.0f;   // Distancia máxima para atenuación (ajustada a 100 unidades)

    void updatePosition();
    void update3DAttributes();
};