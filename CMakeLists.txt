cmake_minimum_required(VERSION 3.20)
project(minesweeper VERSION 0.0.1 LANGUAGES C)

add_executable(minesweeper)

target_sources(minesweeper PRIVATE
  src/main.c
  src/game.c
  src/util.c
)

option(DEBUG "turn on for debug layout" OFF)
if(DEBUG)
  target_compile_definitions(minesweeper PRIVATE DEBUG)
endif()

target_compile_features(minesweeper PRIVATE c_std_99)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_compile_options(minesweeper PRIVATE 
    -Wall 
    -Wextra 
    -Wpedantic 
  )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  target_compile_definitions(minesweeper PRIVATE _CRT_SECURE_NO_WARNINGS)
  target_compile_options(minesweeper PRIVATE 
    -W4
  )
endif()

target_link_libraries(minesweeper PRIVATE
  graphics
  board
)

target_include_directories(minesweeper PRIVATE
  lib/graphics
  lib/board
  include
)

add_subdirectory(${CMAKE_SOURCE_DIR}/lib/tigr)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/graphics)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/board)

add_custom_command(TARGET minesweeper
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} ARGS -E make_directory ${CMAKE_SOURCE_DIR}/minesweeper
  COMMAND ${CMAKE_COMMAND} ARGS -E copy $<TARGET_FILE:minesweeper> ${CMAKE_SOURCE_DIR}/minesweeper
  COMMAND ${CMAKE_COMMAND} ARGS -E copy_directory_if_different ${CMAKE_SOURCE_DIR}/resources ${CMAKE_SOURCE_DIR}/minesweeper/resources
)
