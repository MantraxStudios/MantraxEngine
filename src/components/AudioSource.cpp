#include "AudioSource.h"
#include "../core/AudioManager.h"
#include "GameObject.h"
#include <glm/glm.hpp>
#include <iostream>
#include "../render/Camera.h"
#include "SceneManager.h"

using json = nlohmann::json;

void AudioSource::setOwner(GameObject* owner) {
    Component::setOwner(owner);
    stop(); // Asegurarse de detener cualquier sonido si se cambia el owner
}

void AudioSource::setSound(const std::string& path, bool is3D, bool isLooping, bool isStream) {
    if (channel) {
        stop();
    }

    soundPath = path;
    this->is3D = is3D;
    this->isLooping = isLooping;
    this->isStream = isStream;
}

void AudioSource::setVolume(float vol) {
    volume = glm::clamp(vol, 0.0f, 1.0f);
    if (channel) {
        channel->setVolume(volume);
    }
}

void AudioSource::set3DAttributes(bool enabled) {
    is3D = enabled;
    if (channel) {
        stop();
        shouldPlay = true; // Reproducir de nuevo con los nuevos atributos
    }
}

void AudioSource::setMinDistance(float distance) {
    minDistance = glm::max(0.1f, distance);
    if (channel && is3D) {
        update3DAttributes();
    }
}

void AudioSource::setMaxDistance(float distance) {
    maxDistance = glm::max(minDistance + 0.1f, distance);
    if (channel && is3D) {
        update3DAttributes();
    }
}

void AudioSource::play() {
    if (!soundPath.empty()) {
        stop(); // Detener reproducción anterior si existe

        auto& audioManager = AudioManager::getInstance();

        // Cargar y reproducir el sonido
        FMOD::Sound* sound = audioManager.loadSound(soundPath, is3D, isLooping, isStream);
        if (sound) {
            if (is3D) {
                // Obtener la posición del GameObject
                glm::vec3 pos = owner->getWorldPosition();
                FMOD_VECTOR position = { pos.x, pos.y, pos.z };

                // Reproducir en 3D
                channel = audioManager.playSound(soundPath, position, volume);
                if (channel) {
                    update3DAttributes();
                }
            }
            else {
                // Reproducir en 2D
                channel = audioManager.playSound(soundPath, FMOD_VECTOR{ 0,0,0 }, volume);
            }
        }
    }
}

void AudioSource::stop() {
    if (channel) {
        channel->stop();
        channel = nullptr;
    }
    shouldPlay = false;
}

void AudioSource::pause() {
    if (channel) {
        channel->setPaused(true);
    }
}

void AudioSource::resume() {
    if (channel) {
        channel->setPaused(false);
    }
}

bool AudioSource::isPlaying() const {
    if (channel) {
        bool playing = false;
        channel->isPlaying(&playing);
        return playing;
    }
    return false;
}

bool AudioSource::isPaused() const {
    if (channel) {
        bool paused = false;
        channel->getPaused(&paused);
        return paused;
    }
    return false;
}

void AudioSource::update() {
    if (shouldPlay) {
        play();
        shouldPlay = false;
    }

    if (channel && is3D) {
        updatePosition();
    }
}

void AudioSource::updatePosition() {
    if (owner && channel) {
        glm::vec3 pos = owner->getWorldPosition();
        FMOD_VECTOR position = { pos.x, pos.y, pos.z };
        channel->set3DAttributes(&position, nullptr);
        update3DAttributes();
    }
}

void AudioSource::update3DAttributes() {
    if (channel && is3D) {
        // Actualizar posición
        glm::vec3 pos = owner->getWorldPosition();
        FMOD_VECTOR position = { pos.x, pos.y, pos.z };

        Camera* cam = SceneManager::getInstance().getActiveScene()->getCamera();
        glm::vec3 _velocity = cam->GetVelocity();
        glm::vec3 _front = cam->GetForward();
        glm::vec3 _up = cam->GetUp();

        FMOD_VECTOR velocity = { _velocity.x, _velocity.y, _velocity.z };
        FMOD_VECTOR forward = { _front.x, _front.y, _front.z };
        FMOD_VECTOR up = { _up.x, _up.y, _up.z };

        channel->set3DMinMaxDistance(minDistance, maxDistance);
        FMOD_RESULT result = channel->set3DAttributes(&position, 0);
    }
}


std::string AudioSource::serializeComponent() const {
    json j;
    j["soundPath"] = soundPath;
    j["volume"] = volume;
    j["is3D"] = is3D;
    j["isLooping"] = isLooping;
    j["isStream"] = isStream;
    j["minDistance"] = minDistance;
    j["maxDistance"] = maxDistance;

    return j.dump();
}

void AudioSource::deserialize(const std::string& data) {
    json j = json::parse(data);

    // Extrae y aplica todo usando setters
    std::string path = j.value("soundPath", "");
    bool _is3D = j.value("is3D", true);
    bool _isLooping = j.value("isLooping", false);
    bool _isStream = j.value("isStream", false);

    setSound(path, _is3D, _isLooping, _isStream);
    setVolume(j.value("volume", 1.0f));
    setMinDistance(j.value("minDistance", 5.0f));
    setMaxDistance(j.value("maxDistance", 100.0f));

    // Opcional: restaurar reproducción
    // if (j.value("isPlaying", false)) play();
}
