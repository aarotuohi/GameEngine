#include "AudioManager.h"
#include <iostream>

#ifdef HAS_AUDIO_SUPPORT

AudioManager::AudioManager()
    : backgroundMusic(nullptr),
      masterVolume(75.0f),
      musicVolume(75.0f),
      sfxVolume(75.0f),
      initialized(false),
      musicEnabled(true),
      sfxEnabled(true) {
}

AudioManager::~AudioManager() {
    shutdown();
}

bool AudioManager::initialize() {
    if (initialized) {
        return true;
    }

 
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << std::endl;
        return false;
    }

    Mix_AllocateChannels(16);

    initialized = true;
    std::cout << "AudioManager initialized successfully" << std::endl;
    return true;
}

void AudioManager::shutdown() {
    if (!initialized) {
        return;
    }

  
    if (backgroundMusic) {
        Mix_HaltMusic();
        Mix_FreeMusic(backgroundMusic);
        backgroundMusic = nullptr;
    }

 
    for (auto& pair : soundEffects) {
        if (pair.second) {
            Mix_FreeChunk(pair.second);
        }
    }
    soundEffects.clear();

    Mix_CloseAudio();
    initialized = false;
    std::cout << "AudioManager shut down" << std::endl;
}

bool AudioManager::loadMusic(const std::string& filepath) {
    if (!initialized) {
        std::cerr << "AudioManager not initialized!" << std::endl;
        return false;
    }

 
    if (backgroundMusic) {
        Mix_FreeMusic(backgroundMusic);
        backgroundMusic = nullptr;
    }


    backgroundMusic = Mix_LoadMUS(filepath.c_str());
    if (!backgroundMusic) {
        std::cerr << "Failed to load music: " << filepath << " SDL_mixer Error: " << Mix_GetError() << std::endl;
        return false;
    }

    std::cout << "Loaded music: " << filepath << std::endl;
    updateMusicVolume();
    return true;
}

void AudioManager::playMusic(int loops) {
    if (!initialized || !backgroundMusic || !musicEnabled) {
        return;
    }

    if (Mix_PlayMusic(backgroundMusic, loops) == -1) {
        std::cerr << "Failed to play music! SDL_mixer Error: " << Mix_GetError() << std::endl;
    }
}

void AudioManager::pauseMusic() {
    if (!initialized) {
        return;
    }
    Mix_PauseMusic();
}

void AudioManager::resumeMusic() {
    if (!initialized) {
        return;
    }
    Mix_ResumeMusic();
}

void AudioManager::stopMusic() {
    if (!initialized) {
        return;
    }
    Mix_HaltMusic();
}

bool AudioManager::isMusicPlaying() const {
    if (!initialized) {
        return false;
    }
    return Mix_PlayingMusic() != 0;
}

bool AudioManager::loadSoundEffect(SoundEffect effect, const std::string& filepath) {
    if (!initialized) {
        std::cerr << "AudioManager not initialized!" << std::endl;
        return false;
    }

 
    auto it = soundEffects.find(effect);
    if (it != soundEffects.end() && it->second) {
        Mix_FreeChunk(it->second);
    }


    Mix_Chunk* chunk = Mix_LoadWAV(filepath.c_str());
    if (!chunk) {
        std::cerr << "Failed to load sound effect: " << filepath << " SDL_mixer Error: " << Mix_GetError() << std::endl;
        return false;
    }

    soundEffects[effect] = chunk;
    std::cout << "Loaded sound effect: " << filepath << std::endl;
    updateSFXVolume();
    return true;
}

void AudioManager::playSoundEffect(SoundEffect effect, int loops) {
    if (!initialized || !sfxEnabled) {
        return;
    }

    auto it = soundEffects.find(effect);
    if (it == soundEffects.end() || !it->second) {
        return;
    }


    if (Mix_PlayChannel(-1, it->second, loops) == -1) {
        std::cerr << "Failed to play sound effect! SDL_mixer Error: " << Mix_GetError() << std::endl;
    }
}

void AudioManager::setMasterVolume(float volume) {
    masterVolume = std::max(0.0f, std::min(100.0f, volume));
    updateMusicVolume();
    updateSFXVolume();
}

void AudioManager::setMusicVolume(float volume) {
    musicVolume = std::max(0.0f, std::min(100.0f, volume));
    updateMusicVolume();
}

void AudioManager::setSFXVolume(float volume) {
    sfxVolume = std::max(0.0f, std::min(100.0f, volume));
    updateSFXVolume();
}

void AudioManager::setMusicEnabled(bool enabled) {
    musicEnabled = enabled;
    if (!enabled && isMusicPlaying()) {
        pauseMusic();
    } else if (enabled && backgroundMusic && !isMusicPlaying()) {
        playMusic();
    }
}

void AudioManager::setSFXEnabled(bool enabled) {
    sfxEnabled = enabled;
}

void AudioManager::updateMusicVolume() {
    if (!initialized) {
        return;
    }

    float combinedVolume = (masterVolume / 100.0f) * (musicVolume / 100.0f) * 128.0f;
    Mix_VolumeMusic(static_cast<int>(combinedVolume));
}

void AudioManager::updateSFXVolume() {
    if (!initialized) {
        return;
    }

    float combinedVolume = (masterVolume / 100.0f) * (sfxVolume / 100.0f) * 128.0f;
    int volume = static_cast<int>(combinedVolume);

    for (auto& pair : soundEffects) {
        if (pair.second) {
            Mix_VolumeChunk(pair.second, volume);
        }
    }
}

#else // !HAS_AUDIO_SUPPORT


AudioManager::AudioManager()
    : masterVolume(75.0f),
      musicVolume(75.0f),
      sfxVolume(75.0f),
      initialized(false),
      musicEnabled(true),
      sfxEnabled(true) {}

AudioManager::~AudioManager() {}
bool AudioManager::initialize() { return false; }
void AudioManager::shutdown() {}
bool AudioManager::loadMusic(const std::string&) { return false; }
void AudioManager::playMusic(int) {}
void AudioManager::pauseMusic() {}
void AudioManager::resumeMusic() {}
void AudioManager::stopMusic() {}
bool AudioManager::isMusicPlaying() const { return false; }
bool AudioManager::loadSoundEffect(SoundEffect, const std::string&) { return false; }
void AudioManager::playSoundEffect(SoundEffect, int) {}
void AudioManager::setMasterVolume(float volume) { masterVolume = std::max(0.0f, std::min(100.0f, volume)); }
void AudioManager::setMusicVolume(float volume) { musicVolume = std::max(0.0f, std::min(100.0f, volume)); }
void AudioManager::setSFXVolume(float volume) { sfxVolume = std::max(0.0f, std::min(100.0f, volume)); }
void AudioManager::setMusicEnabled(bool enabled) { musicEnabled = enabled; }
void AudioManager::setSFXEnabled(bool enabled) { sfxEnabled = enabled; }
void AudioManager::updateMusicVolume() {}
void AudioManager::updateSFXVolume() {}

#endif // HAS_AUDIO_SUPPORT
