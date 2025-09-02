add_library(graphics)

target_include_directories(graphics
  PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
  PUBLIC 
    ${CMAKE_SOURCE_DIR}/lib/tigr
)

target_sources(graphics 
  PRIVATE
    src/component.c
    src/panel.c
    src/window.c
    src/mouse_event.c
    src/assets.c
)

target_compile_features(graphics 
  PUBLIC 
    c_std_99
)

target_link_libraries(graphics 
  PUBLIC
    tigr
)

target_compile_definitions(graphics 
  PRIVATE 
    $<$<CONFIG:Debug>:DEBUG>
)

target_compile_options(graphics 
  PRIVATE
    "$<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Wall;-Wextra;-Wpedantic;-O3>"
    $<$<COMPILE_LANG_AND_ID:C,MSVC>:-W3>
)

target_compile_options(graphics 
  PRIVATE
    "$<$<AND:$<CONFIG:Debug>,$<COMPILE_LANG_AND_ID:C,Clang,GNU>>:-g;-fsanitize=address,undefined>"
    $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANG_AND_ID:C,MSVC>>:-fsanitize=address>
)

target_link_options(graphics 
  PRIVATE
    "$<$<AND:$<CONFIG:Debug>,$<COMPILE_LANG_AND_ID:C,Clang,GNU>>:-g;-fsanitize=address,undefined>"
    $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANG_AND_ID:C,MSVC>>:-fsanitize=address>
)
