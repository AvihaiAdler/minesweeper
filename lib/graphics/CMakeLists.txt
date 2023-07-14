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

option(GRAPHICS_DEBUG "debug" OFF)
if(GRAPHICS_DEBUG)
  target_compile_definitions(graphics PRIVATE GRAPHICS_DEBUG)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_compile_options(graphics PRIVATE
    -Wall
    -Wextra
    -Wpedantic
  )
elseif()
  target_compile_options(graphics PRIVATE
    -W4
  )
else()
  message("unsupported system")
endif()
