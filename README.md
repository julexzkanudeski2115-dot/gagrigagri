# adrenaline.win

External ESP + aimbot for Roblox (`RobloxPlayerBeta.exe`). Read-only process
handle, no injection, no DLL, no driver. Draws a transparent Direct3D 11 overlay
over the Roblox window with ImGui.

## Features

- 3D bounding boxes (projected from world space, not a fixed 2D rect)
- Names / display names
- Health bar + health text
- Distance readout
- Head dot
- Snaplines
- Team check (uses the Player's Team instance, compares against the local player)
- Aimbot: FOV circle targeting, nearest-to-crosshair selection, smoothing,
  sensitivity scale, head/root target, hold-key, team check
- ImGui menu (INSERT to toggle), config save/load to `adrenaline.ini`
- Stream proof mode (window excluded from screen capture)

## Build

Requires Visual Studio 2022 (Desktop C++ workload), CMake 3.20+, and Git
(CMake fetches Dear ImGui at configure time).

```
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

Output: `build/Release/adrenaline_win.exe`

## Run

1. Launch Roblox and join a game.
2. Run `adrenaline_win.exe`.
3. INSERT opens/closes the menu. INSERT is the default toggle key; change it in
   the Misc tab.

## Architecture

| File | Responsibility |
|---|---|
| `src/memory.*` | Read-only handle, RPM wrapper, `.text` cache, pattern scanner |
| `src/pattern.hpp` | IDA-style pattern parse + scan |
| `src/offsets.hpp` | **Every offset and signature lives here.** Update this file only. |
| `src/game.*` | DataModel walk: camera, players, characters, humanoids, W2S |
| `src/esp.*` | World-box projection + ImGui draw list rendering |
| `src/aimbot.*` | Target selection + relative mouse input |
| `src/menu.*` | ImGui menu |
| `src/overlay.*` | Transparent topmost D3D11 + ImGui overlay |
| `src/config.hpp` | Settings structs + ini persistence |

## Offsets

`src/offsets.hpp` ships with a known-good set, but **Roblox ships a new client
every few weeks and Hyperion shuffles these values.** When the ESP draws nothing
or draws boxes in the wrong place, re-dump the values and edit only that file.

Where each value comes from:

- `FakeDataModelToDataModel` — deref the FakeDataModel pointer, then read this.
  The FakeDataModel pointer itself is resolved by the signature
  `fakeDataModelSignature` (`48 8B 0D ?? ?? ?? ??`, a RIP-relative `mov rcx`).
- Camera offsets are relative to the `Camera` instance inside `Workspace`.
  `CameraRotation` is a row-major 3x3 (columns are right / up / back);
  `CameraPosition` is a Vector3; `CameraFov` is the vertical FOV.
- Player / Humanoid / BasePart offsets are relative to their instances.

Re-dumping recipe:

1. Attach Cheat Engine to `RobloxPlayerBeta.exe`.
2. Find the `.text` base and scan for the FakeDataModel signature to confirm it
   still resolves to a sane pointer.
3. From the DataModel, walk to `Workspace` → `Camera` and verify the rotation
   matrix at `CameraRotation` changes when you move the mouse.
4. Walk `Players` → a `Player` → `Character` → `HumanoidRootPart` → primitive and
   verify the position tracks the character.
5. Update `src/offsets.hpp`.

## Notes

- External read-only means the handle is opened with `PROCESS_VM_READ |
  PROCESS_QUERY_INFORMATION` only. Nothing is written to the target.
- Hyperion (Byfron) protects the client. External read-only cheats avoid the
  injection path it watches most closely, but no external is undetectable.
  Offsets also rotate on client updates, so expect to maintain this.
- Aimbot moves the real mouse via `SendInput`, so it fights your in-game
  sensitivity. Tune `smooth` and `sensitivity` until tracking feels right.
- If the overlay is misaligned, make sure Roblox is running in windowed or
  borderless — exclusive fullscreen can't be overlapped by a layered window.
