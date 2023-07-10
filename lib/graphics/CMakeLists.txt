add_library(graphics)

target_sources(graphics PRIVATE
  src/component.c
  src/panel.c
  src/window.c
  src/mouse_event.c
)

target_compile_features(graphics PUBLIC c_std_99)

target_include_directories(graphics
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
  PUBLIC ${CMAKE_SOURCE_DIR}/lib/tigr
)

target_link_libraries(graphics PUBLIC
  tigr
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_compile_options(graphics PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -fsanitize=address,undefined
  )

  target_link_options(graphics PRIVATE
    -fsanitize=address,undefined
  )
elseif()
  target_compile_options(graphics PRIVATE
    -W4
  )
else()
  message("unsupported system")
endif()