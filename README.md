### Minesweeper

An implementation of the old but classic windows minesweeper written in C using the [TIGR](https://github.com/erkkah/tigr) library. A special thanks to [CoolBassist](https://github.com/CoolBassist) who introduced the library to me.

#### Controls

As expected the game is played with the mouse. Left click to reveal a tile, right click to mark it with a flag and (due to a design choice) the middle mouse button to reveal all tiles within a marked area

#### Building an compiling

The game uses CMake as its build tool. As such one need it to be installed. Currently the script will work on _linux only_ due to the library `tigr` depends on.

- clone the project
- navigate to the project directory
- `cmake -S . -B build` (add `-G <generator>` if you prefer to use anything other than `make`)
- `cmake --build build` to build the project
- the binary (`minesweeper`) will be located under `build/`

#### RoadMap

- [ ] add the ability to mark a tile with a question mark
- [ ] add the ability to change difficulty
- [ ] support for windows

#### Notes

The pixelart was done by me - feel free to use it should you want to 