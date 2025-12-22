# Audio Assets

This directory contains audio files for the game.

## Music
Place your background music files in `music/`:
- Supported formats: MP3, OGG, WAV, FLAC
- Recommended: OGG format for best quality/size ratio
- Example: `music/background.ogg`

## Sound Effects
Place your sound effect files in `sounds/`:
- Supported formats: WAV, OGG
- Recommended: WAV format for low-latency sound effects
- Example files:
  - `sounds/ability_q.wav` - Steel Tempest ability
  - `sounds/ability_w.wav` - Wind Wall ability
  - `sounds/ability_e.wav` - Shockwave ability
  - `sounds/ability_r.wav` - Circulating Tornadoes ability
  - `sounds/player_hit.wav` - Player takes damage
  - `sounds/player_death.wav` - Player dies
  - `sounds/enemy_hit.wav` - Enemy takes damage
  - `sounds/enemy_death.wav` - Enemy dies
  - `sounds/level_up.wav` - Player levels up
  - `sounds/sword_swing.wav` - Sword attack

## Free Audio Resources

You can find free game audio at:
- OpenGameArt.org
- Freesound.org
- Incompetech.com (music)
- Zapsplat.com
- Sonniss.com (GDC bundles)

## Loading Audio in Code

To load background music in GameClient.cpp, uncomment these lines in the `connect()` function:

```cpp
audioManager->loadMusic("assets/music/background.ogg");
audioManager->playMusic(); // -1 for infinite loop
```

To load sound effects, add in the `connect()` function:

```cpp
audioManager->loadSoundEffect(SoundEffect::ABILITY_Q, "assets/sounds/ability_q.wav");
audioManager->loadSoundEffect(SoundEffect::ABILITY_W, "assets/sounds/ability_w.wav");
audioManager->loadSoundEffect(SoundEffect::ABILITY_E, "assets/sounds/ability_e.wav");
audioManager->loadSoundEffect(SoundEffect::ABILITY_R, "assets/sounds/ability_r.wav");
audioManager->loadSoundEffect(SoundEffect::PLAYER_HIT, "assets/sounds/player_hit.wav");
audioManager->loadSoundEffect(SoundEffect::PLAYER_DEATH, "assets/sounds/player_death.wav");
audioManager->loadSoundEffect(SoundEffect::ENEMY_HIT, "assets/sounds/enemy_hit.wav");
audioManager->loadSoundEffect(SoundEffect::ENEMY_DEATH, "assets/sounds/enemy_death.wav");
audioManager->loadSoundEffect(SoundEffect::LEVEL_UP, "assets/sounds/level_up.wav");
audioManager->loadSoundEffect(SoundEffect::SWORD_SWING, "assets/sounds/sword_swing.wav");
```
