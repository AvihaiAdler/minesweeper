add_library(graphics)

target_include_directories(graphics
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
  PUBLIC ${CMAKE_SOURCE_DIR}/lib/tigr
)

target_sources(graphics PRIVATE
  src/component.c
  src/panel.c
  src/window.c
  src/mouse_event.c
  src/assets.c
)

target_compile_features(graphics PUBLIC c_std_99)

target_link_libraries(graphics PUBLIC
  tigr
)

option(DEBUG "debug" OFF)
if(DEBUG)
  target_compile_definitions(graphics PRIVATE DEBUG)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_compile_options(graphics PRIVATE
    -O3
    -Wall
    -Wextra
    -Wpedantic
  )
  if(DEBUG)
  target_compile_options(graphics PRIVATE
    -g
    -Og
    -fsanitize=address,undefined
  )
  target_link_options(graphics PRIVATE
    -fsanitize=address,undefined
  )
endif()
elseif()
  target_compile_options(graphics PRIVATE
    -W4
  )
else()
  message("unsupported system")
endif()
