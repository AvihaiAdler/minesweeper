add_library(board)

target_include_directories(board 
  PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_sources(board 
  PRIVATE
    board.c
)

target_compile_features(board 
  PRIVATE 
    c_std_99
)

target_compile_options(board 
  PRIVATE
    "$<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Wall;-Wextra;-Wpedantic;-O3>"
    $<$<COMPILE_LANG_AND_ID:C,MSVC>:-W4>
)