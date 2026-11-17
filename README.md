# SHADOWBLADE: ECHOES OF DUSK

A 2D action-platformer prototype built with **C++17** and **raylib**. Explore a twilight ruin, jump across platforms, dash through gaps, collect luminous echoes, avoid roaming shadow sentries, and reach the exit.

## Current prototype
- Responsive platform movement, acceleration, friction, variable-height jump and coyote-time forgiveness
- Short cooldown-based dash
- Platform collision and falling reset
- Roaming sentries that temporarily reduce the player's energy on contact (non-graphic)
- Collectible echo shards, checkpoint and exit gate
- Title, pause, game-over and victory states
- Keyboard controls and on-screen touch controls
- Procedural vector-style scenery drawn with raylib; no third-party art required

## Controls
| Action | Keyboard | Touch |
|---|---|---|
| Move | A / D or arrows | Left / right |
| Jump | Space / W / Up | JUMP |
| Dash | Left Shift / X | DASH |
| Pause | Escape | Pause button |

## Build on desktop
Requirements: CMake 3.20+, C++17 compiler, Git and an internet connection on the first configure (CMake downloads raylib).

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
./build/Shadowblade
```

On Windows, run `build/Release/Shadowblade.exe` (or the executable directly under `build`, depending on the generator).

## Project layout
```text
Shadowblade-2D/
├── .github/workflows/desktop-build.yml
├── assets/
│   ├── README.md
│   └── shadowblade-mark.svg
├── src/main.cpp
├── CMakeLists.txt
├── .gitignore
└── README.md
```

## Android status
The gameplay input includes raylib touch handling, but this repository currently provides a desktop CMake build configuration, not a ready-to-install Android APK. Android packaging needs a separate Android toolchain/template and must be tested on a device before an APK can be called complete.

## Notes
- All artwork in `assets/` is original project artwork.
- Gameplay focuses on platforming and obstacle avoidance, not weapon-based combat.
- This is an early playable foundation; audio, frame-based character animation, additional stages and Android packaging are follow-up work.
