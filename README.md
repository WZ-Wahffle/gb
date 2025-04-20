# gb: a best-effort GameBoy Color emulator written in (just over) 2 months

This emulator supports both GameBoy and GameBoy Color ROMs, with the former also benefiting from the GameBoy Color's palette enhancements.

## how to compile
The entire emulator is built around Raylib with the SDL backend, using ImGui as a debugging view. All dependencies are included as Git submodules, so no libraries required. Auxiliary programs needed are `gcc` and `g++`, as well as basic coreutils/binutils. For all needed commands, check the `build.sh`.
```
git clone --recursive https://github.com/WZ-Wahffle/gb.git
cd gb
./build.sh
```

## how to run
The build process produces a statically linked executable, meaning that (ideally) no runtime dependencies must be met. In order to use, simply point it to a GameBoy or GameBoy Color ROM file.
```
./gb /path/to/rom.gb
./gb /path/to/other/rom.gbc
```
Once the window has opened, press `TAB` to bring up the debug UI, and press `Start` in order to start execution. Afterward, it can be halted with `Stop` or stepped, instruction by instruction, via `Step`.

## controls

Aside from these keyboard bindings, this emulator also has full support for a USB controller. As such, both the D-Pad and the left analog stick control the GameBoy's D-Pad, the right face buttons are for A and B and the center buttons are mapped to Start and Select.

| GameBoy input | keyboard keybind |
|---------------|------------------|
| `D-Pad`       | arrow keys       |
| `A`           | a                |
| `B`           | b                |
| `Start`       | RShift, Enter    |
| `Select`      | LShift           |

## save files
Some real game cartridges may provide battery-backed persistent memory. In order to emulate this, the emulator contains a `Save` and `Load` button, which automatically save to / load from a `.wsav` file, which will be located in the same directory as the source ROM. Loading save data should ideally be done before pressing `Start`, since some games load their persistent memory very early on during execution.
It is also worth mentioning that if a game does not normally support persistent data, these save files are likely entirely pointless and may potentially serve to break the game on reboot. Can't say I recommend it.
