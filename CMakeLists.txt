cmake_minimum_required(VERSION 3.20)
project(minesweeper VERSION 0.0.1 LANGUAGES C )

add_executable(minesweeper)

target_sources(minesweeper PRIVATE
  src/main.c
  src/util.c
  src/board.c
  src/panel.c
  src/window.c
)

option(DEBUG "turn on for debug layout" OFF)
if(DEBUG)
  target_compile_definitions(minesweeper PRIVATE MS_DEBUG)
endif()

target_compile_features(minesweeper PRIVATE c_std_99)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_compile_options(minesweeper PRIVATE 
    -Wall 
    -Wextra 
    -Wpedantic 
    -g
    -Og
    -fsanitize=address,undefined
  )

  target_link_options(minesweeper PRIVATE
    -fsanitize=address,undefined
  )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  target_compile_options(minesweeper PRIVATE 
    -W4
  )
endif()

target_link_libraries(minesweeper PRIVATE
  tigr
)

target_include_directories(minesweeper PRIVATE
  lib
  include
)

add_subdirectory(${CMAKE_SOURCE_DIR}/lib)