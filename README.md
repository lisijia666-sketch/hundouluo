# Arcade Run Gun

C++17 + SDL2 original arcade run-and-gun game. It uses generated pixel-style drawing instead of Contra assets, with fixed 60 FPS update, side scrolling, platforming, 8-way aiming, enemies, turrets, particles, spread shot, and a boss fight.

## Controls

- `A / D` or arrow keys: move
- `W / S` or arrow keys: aim up/down
- `J` or `Space`: jump
- `K` or `Ctrl`: fire
- `1`: rifle
- `2`: spread shot
- `Q`: toggle rifle/spread
- `R`: restart after death or victory
- `Esc`: quit

The default weapon is spread shot, so press `K` to fire spread shots immediately.

## Run

```powershell
.\run_game.bat
```

Or run:

```powershell
.\build\arcade_run_gun.exe
```

## Build

This workspace already contains a portable MSYS2 toolchain under `tools\msys64`.

```powershell
$env:PATH='G:\hundouluo\tools\msys64\ucrt64\bin;G:\hundouluo\tools\msys64\usr\bin;' + $env:PATH
tools\msys64\ucrt64\bin\cmake.exe -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
tools\msys64\ucrt64\bin\cmake.exe --build build --config Release
```
