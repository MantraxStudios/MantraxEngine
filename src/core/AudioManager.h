#pragma once
#include <fmod/fmod.hpp>
#include <string>
#include <unordered_map>
#include <memory>
#include "../core/CoreExporter.h"

class MANTRAXCORE_API AudioManager {
public:
    static AudioManager& getInstance();
    static void destroy();

    bool initialize(int maxChannels = 32);
    void update();
    void shutdown();

    // Carga y gestión de sonidos
    FMOD::Sound* loadSound(const std::string& path, bool is3D = false, bool isLooping = false, bool isStream = false);
    void unloadSound(const std::string& path);
    void unloadAllSounds();

    // Reproducción de sonidos
    FMOD::Channel* playSound(const std::string& path, const FMOD_VECTOR& position = {0,0,0}, float volume = 1.0f);
    FMOD::Channel* playSoundWithChannel(const std::string& path, int channelId, const FMOD_VECTOR& position = {0,0,0}, float volume = 1.0f);

    // Control de canales
    void stopChannel(int channelId);
    void stopAllChannels();
    void setChannelVolume(int channelId, float volume);
    void setChannelPosition(int channelId, const FMOD_VECTOR& position);

    // Control de listener (para audio 3D)
    void setListenerPosition(const FMOD_VECTOR& position, const FMOD_VECTOR* velocity,
                           const FMOD_VECTOR* forward, const FMOD_VECTOR* up);

    // Configuración 3D
    void set3DSettings(float dopplerScale, float distanceFactor, float rolloffScale);
    void get3DSettings(float* dopplerScale, float* distanceFactor, float* rolloffScale);

private:
    AudioManager() = default;
    ~AudioManager() = default;
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    static AudioManager* instance;
    FMOD::System* system = nullptr;
    std::unordered_map<std::string, FMOD::Sound*> sounds;
    std::unordered_map<int, FMOD::Channel*> channels;

    bool checkError(FMOD_RESULT result);
}; 