add_library(board)

target_include_directories(board PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(board PRIVATE
  board.c
)

target_compile_features(board PRIVATE c_std_99)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_compile_options(board PRIVATE
    -O3
    -Wall
    -Wextra
    -Wpedantic
  )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  target_compile_options(board PRIVATE
    -W4
  )
else()
  message("unsupported system")
endif()