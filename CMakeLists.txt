cmake_minimum_required(VERSION 3.26)
project(minesweeper VERSION 0.1.0 LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(minesweeper)

target_sources(minesweeper PRIVATE
  src/main.c
  src/game.c
  src/util.c
)

target_compile_features(minesweeper 
  PRIVATE 
    c_std_99
)

target_compile_definitions(minesweeper 
  PRIVATE 
    $<$<CONFIG:Debug>:DEBUG>
)

target_compile_definitions(minesweeper 
  PRIVATE
    $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
)

target_compile_options(minesweeper 
  PRIVATE
    "$<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Wall;-Wextra;-Wpedantic>"
    $<$<COMPILE_LANG_AND_ID:C,MSVC>:-W3>
)

target_compile_options(minesweeper 
  PRIVATE
    "$<$<AND:$<CONFIG:Debug>,$<COMPILE_LANG_AND_ID:C,Clang,GNU>>:-g;-fsanitize=address,undefined>"
    $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANG_AND_ID:C,MSVC>>:-fsanitize=address>
)

target_link_options(minesweeper 
  PRIVATE
    "$<$<AND:$<CONFIG:Debug>,$<COMPILE_LANG_AND_ID:C,Clang,GNU>>:-g;-fsanitize=address,undefined>"
    $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANG_AND_ID:C,MSVC>>:-fsanitize=address>
)


target_link_libraries(minesweeper 
  PRIVATE
    graphics
    board
)

target_include_directories(minesweeper 
  PRIVATE
    lib/graphics
    lib/board
    include
)

add_subdirectory(${CMAKE_SOURCE_DIR}/lib/tigr)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/graphics)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/board)

install(TARGETS minesweeper
  RUNTIME
    DESTINATION ${CMAKE_SOURCE_DIR}/bin
)

install(DIRECTORY resources
    DESTINATION ${CMAKE_SOURCE_DIR}/bin
)
