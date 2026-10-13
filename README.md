<div align="center">

# Crossy Road 2D

**An endless arcade survival game built from scratch in modern C++ and SFML 3.**

![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?logo=cplusplus&logoColor=white)
![SFML](https://img.shields.io/badge/SFML-3.1.0-8CC445)
![CMake](https://img.shields.io/badge/CMake-3.15%2B-064F8C?logo=cmake&logoColor=white)
![Platform](https://img.shields.io/badge/platform-Windows%2010%2F11-0078D6?logo=windows&logoColor=white)
![License](https://img.shields.io/badge/license-MIT-green)

[Demo video](https://youtu.be/5cUIjrElCeg) · [Report (PDF)](report.pdf) · [Slides (PDF)](slide.pdf)

</div>

---

## Overview

Guide your character across an endless map of grass fields, busy roads and rivers. Dodge cars and trucks, hop onto floating logs, collect coins and unlock new character skins, all while the world is generated procedurally as you move forward.

The project was developed as an Object-Oriented Programming course project at HCMUS. It focuses on clean OOP design, design patterns and a robust game architecture rather than relying on a game engine.

## Features

- **Endless procedural map** - lanes are generated ahead of the camera and removed once they scroll away; difficulty rises with the score (every 10 points is one level).
- **Four lane types** - grass, road, river and safe zone, each with its own obstacles and rules.
- **Rich obstacles and objects** - cars, trucks, birds, rabbits, rocks, floating wood, traffic lights that cycle through red, yellow and green on randomized timers.
- **Coin and shop system** - collect coins and spend them to buy and select one of 3 character skins.
- **Save / Load** - 3 save slots that show the player name and score; data is stored in an encrypted binary format.
- **High scores** - persistent top-5 ranking.
- **Settings** - separate music / SFX volume and an auto-scroll camera mode.
- **Polished UX** - pause overlay, loading screen, hover/click audio feedback, help and credits screens.

## Tech Stack

|                          |                                                    |
| ------------------------ | -------------------------------------------------- |
| Language                 | C++17                                              |
| Graphics / Audio / Input | SFML 3.1.0                                         |
| Build system             | CMake (3.15+)                                      |
| Toolchains               | MSVC (Visual Studio 2022) or MinGW-w64 (GCC 14.2+) |
| Platform                 | Windows 10/11 (64-bit)                             |

## Architecture Highlights

- **State pattern + State machine** - every screen (home, playing, pause, shop, ranking...) is a `State`; `StateMachine` manages a stack with `push`, `pop`, `switch` and `reset` transitions, deferred until the start of the next frame for safety. Overlay states (e.g. pause) render on top of the previous one.
- **Polymorphic entity hierarchy** - `DynamicObject` (vehicles, animals, wood, player) and `StaticObject` (rocks, coins, traffic lights) expose a common interface for update, render and collision handling.
- **Simple factory** - `Map::createLane` builds the right `Lane` subclass (`LaneGrass`, `LaneRoad`, `LaneRiver`, `LaneSafe`) for each lane type.
- **Sliding window of lanes** - the map keeps its lanes in a `std::deque`, pushing new lanes at the top and popping old ones at the bottom as the camera moves.
- **Shared context** - a lightweight `SharedContext` passes the window, config and player data between states.
- **Resource and audio management** - centralized `ResourceManager` (textures, fonts) and `AudioSystem` (sound effects, music).
- **Binary serialization** - templated read/write helpers with XOR cipher for save files and player data.
- **Custom exception hierarchy** - `GameException` base class with dedicated errors for asset loading (texture, font, audio) and state-machine misuse.
- **RAII and smart pointers** - `std::unique_ptr` ownership throughout, no manual `new` / `delete`.

## Project Structure

```text
.
├── assets/            # Textures, fonts and audio
├── data/              # Text screens, high scores, player data and save slots
├── include/           # Headers
│   ├── core/          #   Game loop, state machine
│   ├── entities/      #   Player, vehicles, animals, coins, traffic lights
│   ├── states/        #   Game screens (home, playing, pause, shop...)
│   ├── utils/         #   Constants, resources, audio, serialization, exceptions
│   └── world/         #   Map and lane types
├── src/               # Implementations (mirrors include/)
├── libs/              # Bundled SFML 3.1.0 (MSVC and MinGW builds)
├── CMakeLists.txt
└── CMakeSettings.json
```

## Getting Started

### Prerequisites

- Windows 10/11 (64-bit)
- **Visual Studio 2022** with _Desktop development with C++_ and _C++ CMake tools for Windows_,
  **or** MinGW-w64 (GCC 14.2+) with CMake 3.15+

SFML 3.1.0 is bundled in `libs/`; no extra installation is required.

### Build and run with Visual Studio 2022

1. Open Visual Studio and choose **Open a local folder**, then select the repository root.
2. Wait for CMake generation to finish.
3. Select the `x64-Debug` or `x64-Release` configuration and the `CrossyRoad.exe` startup item.
4. Press **F5**.

### Build and run with CMake and MinGW

```bash
git clone https://github.com/tuongvii2327/OOP_04_crossing_game.git
cd OOP_04_crossing_game

cmake -B build -G "MinGW Makefiles"
cmake --build build

./build/CrossyRoad.exe
```

The build automatically copies `assets/`, `data/` and the SFML DLLs next to the executable.

## Team

**Group 04 - Object-Oriented Programming, HCMUS**

- Nguyen Ba Duy
- Nguyen Duy Khang
- Trinh Tran Huong Mai
- Tran Ngoc Nghia
- Thai Mac Tuong Vi

## Acknowledgements

- Dr. Truong Toan Thinh for guidance and support throughout the project.
- The [SFML](https://www.sfml-dev.org/) community and its open-source resources.
- Font: [VT323](https://fonts.google.com/specimen/VT323).

## License

Distributed under the MIT License. See [LICENSE](LICENSE) for details.
