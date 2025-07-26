#include "AudioManager.h"
#include <iostream>
#include "../components/SceneManager.h"
#include "../render/Camera.h"

AudioManager* AudioManager::instance = nullptr;

AudioManager& AudioManager::getInstance() {
    if (!instance) {
        instance = new AudioManager();
    }
    return *instance;
}

void AudioManager::destroy() {
    if (instance) {
        instance->shutdown();
        delete instance;
        instance = nullptr;
    }
}

bool AudioManager::initialize(int maxChannels) {
    FMOD_RESULT result;
    
    // Crear el sistema FMOD
    result = FMOD::System_Create(&system);
    if (!checkError(result)) return false;

    // Inicializar FMOD con configuración 3D
    result = system->init(maxChannels, FMOD_INIT_NORMAL | FMOD_INIT_3D_RIGHTHANDED, nullptr);
    if (!checkError(result)) return false;

    // Configurar parámetros 3D
    // - Doppler scale: 1.0 = efecto Doppler normal
    // - Distance factor: 1.0 = 1 unidad = 1 metro
    // - Rolloff scale: 0.75 = atenuación más suave
    result = system->set3DSettings(1.0f, 1.0f, 0.75f);
    if (!checkError(result)) return false;

    // Configurar el listener inicial
    // Nota: Estos valores se actualizarán en el primer frame con la posición real de la cámara
    FMOD_VECTOR initialPos = { 0.0f, 0.0f, 0.0f };
    FMOD_VECTOR initialVel = { 0.0f, 0.0f, 0.0f };
    FMOD_VECTOR initialForward = { 0.0f, 0.0f, 1.0f };
    FMOD_VECTOR initialUp = { 0.0f, 1.0f, 0.0f };
    result = system->set3DListenerAttributes(0, &initialPos, &initialVel, &initialForward, &initialUp);
    if (!checkError(result)) return false;

    std::cout << "FMOD initialized successfully with 3D audio settings" << std::endl;
    std::cout << "Max channels: " << maxChannels << std::endl;
    std::cout << "3D Settings - Doppler: 1.0, Distance Factor: 1.0, Rolloff: 0.75" << std::endl;
    return true;
}

void AudioManager::update() {
    if (system) {
        // Obtener la cámara activa
        auto* scene = SceneManager::getInstance().getActiveScene();
        if (scene && scene->getCamera()) {
            Camera* camera = scene->getCamera();
            
            // Obtener los vectores de la cámara
            glm::vec3 position = camera->getPosition();
            glm::vec3 velocity = camera->GetVelocity();
            glm::vec3 forward = camera->GetForward();
            glm::vec3 up = camera->GetUp();

            // Convertir a FMOD_VECTOR
            FMOD_VECTOR fmodPosition = { position.x, position.y, position.z };
            FMOD_VECTOR fmodVelocity = { velocity.x, velocity.y, velocity.z };
            FMOD_VECTOR fmodForward = { forward.x, forward.y, forward.z };
            FMOD_VECTOR fmodUp = { up.x, up.y, up.z };

            // Actualizar atributos del listener
            system->set3DListenerAttributes(0, &fmodPosition, &fmodVelocity, &fmodForward, &fmodUp);

            // Debug info
            static float debugTimer = 0.0f;
            debugTimer += 0.1f; // Aproximadamente cada 0.1 segundos
            if (debugTimer >= 1.0f) {
                std::cout << "Audio Listener - Position: (" 
                          << position.x << ", " << position.y << ", " << position.z 
                          << ") Forward: (" 
                          << forward.x << ", " << forward.y << ", " << forward.z << ")\n";
                debugTimer = 0.0f;
            }
        }

        // Actualizar el sistema FMOD
        system->update();
    }
}

void AudioManager::shutdown() {
    if (system) {
        unloadAllSounds();
        system->close();
        system->release();
        system = nullptr;
    }
}

FMOD::Sound* AudioManager::loadSound(const std::string& path, bool is3D, bool isLooping, bool isStream) {
    // Verificar si el sonido ya está cargado
    auto it = sounds.find(path);
    if (it != sounds.end()) {
        return it->second;
    }

    FMOD::Sound* sound = nullptr;
    FMOD_MODE mode = FMOD_DEFAULT;
    
    // Configurar modo según parámetros
    if (is3D) {
        mode = FMOD_3D | FMOD_3D_LINEARROLLOFF; // Forzar modo 3D y atenuación lineal
        std::cout << "Loading sound '" << path << "' in 3D mode" << std::endl;
    } else {
        mode = FMOD_2D;
        std::cout << "Loading sound '" << path << "' in 2D mode" << std::endl;
    }

    if (isLooping) {
        mode |= FMOD_LOOP_NORMAL;
    } else {
        mode |= FMOD_LOOP_OFF;
    }

    if (isStream) {
        mode |= FMOD_CREATESTREAM;
    }

    // Cargar el sonido
    FMOD_RESULT result = system->createSound(path.c_str(), mode, nullptr, &sound);
    if (!checkError(result)) {
        std::cerr << "Failed to load sound: " << path << std::endl;
        return nullptr;
    }

    // Verificar que el modo 3D se aplicó correctamente
    if (is3D) {
        FMOD_MODE currentMode;
        sound->getMode(&currentMode);
        if (!(currentMode & FMOD_3D)) {
            std::cerr << "Warning: Sound was requested as 3D but mode is not 3D!" << std::endl;
        }
    }

    sounds[path] = sound;
    return sound;
}

void AudioManager::unloadSound(const std::string& path) {
    auto it = sounds.find(path);
    if (it != sounds.end()) {
        it->second->release();
        sounds.erase(it);
    }
}

void AudioManager::unloadAllSounds() {
    for (auto& pair : sounds) {
        if (pair.second) {
            pair.second->release();
        }
    }
    sounds.clear();
    channels.clear();
}

FMOD::Channel* AudioManager::playSound(const std::string& path, const FMOD_VECTOR& position, float volume) {
    FMOD::Sound* sound = loadSound(path);
    if (!sound) return nullptr;

    FMOD::Channel* channel = nullptr;
    FMOD_RESULT result = system->playSound(sound, nullptr, false, &channel);
    if (!checkError(result)) return nullptr;

    if (channel) {
        channel->setVolume(volume);
        channel->set3DAttributes(&position, nullptr);
    }

    return channel;
}

FMOD::Channel* AudioManager::playSoundWithChannel(const std::string& path, int channelId, const FMOD_VECTOR& position, float volume) {
    FMOD::Sound* sound = loadSound(path);
    if (!sound) return nullptr;

    FMOD::Channel* channel = nullptr;
    FMOD_RESULT result = system->playSound(sound, nullptr, false, &channel);
    if (!checkError(result)) return nullptr;

    if (channel) {
        channel->setVolume(volume);
        channel->set3DAttributes(&position, nullptr);
        channels[channelId] = channel;
    }

    return channel;
}

void AudioManager::stopChannel(int channelId) {
    auto it = channels.find(channelId);
    if (it != channels.end() && it->second) {
        it->second->stop();
    }
}

void AudioManager::stopAllChannels() {
    for (auto& pair : channels) {
        if (pair.second) {
            pair.second->stop();
        }
    }
    channels.clear();
}

void AudioManager::setChannelVolume(int channelId, float volume) {
    auto it = channels.find(channelId);
    if (it != channels.end() && it->second) {
        it->second->setVolume(volume);
    }
}

void AudioManager::setChannelPosition(int channelId, const FMOD_VECTOR& position) {
    auto it = channels.find(channelId);
    if (it != channels.end() && it->second) {
        it->second->set3DAttributes(&position, nullptr);
    }
}

void AudioManager::setListenerPosition(const FMOD_VECTOR& position, const FMOD_VECTOR* velocity,
                                     const FMOD_VECTOR* forward, const FMOD_VECTOR* up) {
    if (system) {
        system->set3DListenerAttributes(0, &position, velocity, forward, up);
    }
}

void AudioManager::set3DSettings(float dopplerScale, float distanceFactor, float rolloffScale) {
    if (system) {
        system->set3DSettings(dopplerScale, distanceFactor, rolloffScale);
    }
}

void AudioManager::get3DSettings(float* dopplerScale, float* distanceFactor, float* rolloffScale) {
    if (system) {
        system->get3DSettings(dopplerScale, distanceFactor, rolloffScale);
    }
}

bool AudioManager::checkError(FMOD_RESULT result) {
    if (result != FMOD_OK) {
        std::cerr << "FMOD Error: " << result << std::endl;
        return false;
    }
    return true;
} 