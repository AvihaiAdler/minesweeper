### Minesweeper

An implementation of the old but classic windows minesweeper written in C using the [TIGR](https://github.com/erkkah/tigr) library. A special thanks to [CoolBassist](https://github.com/CoolBassist) who introduced the library to me.

#### Controls

As expected the game is played with the mouse. Left click to reveal a tile, right click to mark it with a flag and (due to a design choice) the middle mouse button to reveal all tiles within a marked area

#### Building an compiling

The game uses CMake as its build tool. As such one need it to be installed.

#### Prerequisites:
##### Linux users
- Make sure you have `freeglut3-dev` installed

##### Windows users
- Make sure you have OpenGL installed 

##### Mac users
- Sorry - you're on your own here

##### WSL
`Tigr` struggle when in comes to WSL. The game will compile and will work just fine in Debug mode - however it won't in Release mode. Moreover the program will seg-fault on termination

- clone the project
- navigate to the project directory
- `cmake -S . -B build` (add `-G <generator>` if you prefer to use anything other than `make`)
- `cmake --build build` to build the project
- the game and all its files will be under `minesweeper/`


#### RoadMap

- [ ] add the ability to mark a tile with a question mark
- [x] add the ability to change difficulty
- [x] support for windows
- [ ] fix an alignment issue when opting for full screen

#### Notes

The pixelart was done by me - feel free to use it should you want to 
