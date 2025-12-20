#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#ifdef HAS_AUDIO_SUPPORT
#include <SDL2/SDL_mixer.h>
#endif
#include <string>
#include <unordered_map>

enum class SoundEffect {
    PLAYER_HIT,
    PLAYER_DEATH,
    ABILITY_Q,
    ABILITY_W,
    ABILITY_E,
    ABILITY_R,
    ENEMY_HIT,
    ENEMY_DEATH,
    LEVEL_UP,
    SWORD_SWING
};

class AudioManager {
private:
#ifdef HAS_AUDIO_SUPPORT
    Mix_Music* backgroundMusic;
    std::unordered_map<SoundEffect, Mix_Chunk*> soundEffects;
#endif
    
    float masterVolume;
    float musicVolume;
    float sfxVolume;
    
    bool initialized;
    bool musicEnabled;
    bool sfxEnabled;

public:
    AudioManager();
    ~AudioManager();

   
    bool initialize();
    
   
    void shutdown();
    
    
    bool loadMusic(const std::string& filepath);
    void playMusic(int loops = -1); 
    void pauseMusic();
    void resumeMusic();
    void stopMusic();
    bool isMusicPlaying() const;
    
    
    bool loadSoundEffect(SoundEffect effect, const std::string& filepath);
    void playSoundEffect(SoundEffect effect, int loops = 0);
    
    
    void setMasterVolume(float volume);
    void setMusicVolume(float volume);
    void setSFXVolume(float volume);
    
    float getMasterVolume() const { return masterVolume; }
    float getMusicVolume() const { return musicVolume; }
    float getSFXVolume() const { return sfxVolume; }
 
    void setMusicEnabled(bool enabled);
    void setSFXEnabled(bool enabled);
    
    bool isMusicEnabled() const { return musicEnabled; }
    bool isSFXEnabled() const { return sfxEnabled; }
    
    bool isInitialized() const { return initialized; }

private:
    void updateMusicVolume();
    void updateSFXVolume();
};

#endif // AUDIOMANAGER_H
