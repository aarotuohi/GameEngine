# Adding Audio to Your Game - Quick Start Guide

Audio support has been added to your game using SDL2_mixer! Here's how to set it up:

## 1. Install SDL2_mixer

### Windows (vcpkg - Recommended)
```powershell
vcpkg install sdl2-mixer:x64-windows
```

### Ubuntu/Debian
```bash
sudo apt-get install libsdl2-mixer-dev
```

### MacOS
```bash
brew install sdl2_mixer
```

## 2. Add Your Audio Files

1. Download or create your audio files
2. Place background music in `assets/music/` (MP3, OGG, WAV, or FLAC)
3. Place sound effects in `assets/sounds/` (WAV recommended for low latency)

### Free Audio Resources:
- **OpenGameArt.org** - Game assets including audio
- **Freesound.org** - Community sound effects
- **Incompetech.com** - Royalty-free music by Kevin MacLeod
- **Zapsplat.com** - Sound effects library
- **Sonniss.com** - Professional GDC audio bundles (free annually)

## 3. Load Audio Files in Code

Edit `client/GameClient.cpp`, find the `connect()` function, and uncomment/add:

```cpp
#ifdef HAS_AUDIO_SUPPORT
    if (audioManager && audioManager->initialize()) {
        std::cout << "Audio system initialized\n";
        
        // Load background music
        if (audioManager->loadMusic("assets/music/background.ogg")) {
            audioManager->playMusic(); // -1 for infinite loop
        }
        
        // Load sound effects
        audioManager->loadSoundEffect(SoundEffect::ABILITY_Q, "assets/sounds/ability_q.wav");
        audioManager->loadSoundEffect(SoundEffect::ABILITY_W, "assets/sounds/ability_w.wav");
        audioManager->loadSoundEffect(SoundEffect::ABILITY_E, "assets/sounds/ability_e.wav");
        audioManager->loadSoundEffect(SoundEffect::ABILITY_R, "assets/sounds/ability_r.wav");
        audioManager->loadSoundEffect(SoundEffect::SWORD_SWING, "assets/sounds/sword_swing.wav");
        audioManager->loadSoundEffect(SoundEffect::PLAYER_HIT, "assets/sounds/player_hit.wav");
        audioManager->loadSoundEffect(SoundEffect::LEVEL_UP, "assets/sounds/level_up.wav");
    }
#endif
```

## 4. Rebuild Your Project

```powershell
cd build
cmake ..
cmake --build . --config Debug
```

If SDL2_mixer is not found, the game will build without audio support (graceful fallback).

## 5. Control Volume

The game already has volume controls in the settings menu (gear icon):
- **Master Volume** - Overall volume control
- **Music Volume** - Background music volume
- **SFX Volume** - Sound effects volume

## Features Included

✅ Background music playback with looping
✅ Sound effects for abilities (Q, W, E, R)
✅ Volume controls (Master, Music, SFX)
✅ Play/pause/stop music controls
✅ Graceful fallback if SDL2_mixer not installed
✅ Sound effects already integrated for abilities

## Sound Effect Triggers

Sound effects are automatically played when:
- **Q Ability** (Steel Tempest) - SoundEffect::ABILITY_Q
- **W Ability** (Wind Wall) - SoundEffect::ABILITY_W
- **E Ability** (Shockwave) - SoundEffect::ABILITY_E
- **R Ability** (Tornadoes) - SoundEffect::ABILITY_R

You can add more sound effects for:
- `PLAYER_HIT` - When player takes damage
- `PLAYER_DEATH` - When player dies
- `ENEMY_HIT` - When enemy takes damage
- `ENEMY_DEATH` - When enemy dies
- `LEVEL_UP` - When player levels up
- `SWORD_SWING` - Basic attack sound

## Next Steps

1. Find or create your audio files
2. Place them in `assets/music/` and `assets/sounds/`
3. Update the file paths in `GameClient.cpp`
4. Rebuild and test!

Enjoy your game with audio! 🎵🎮
