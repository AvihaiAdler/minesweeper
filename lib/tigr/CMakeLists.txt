add_library(tigr STATIC)

target_sources(tigr PRIVATE
  tigr.c
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  find_library(GLU
    NAMES GLU
    REQUIRED
  )

  find_library(X11
    NAMES X11
    REQUIRED
  )

  find_library(GL
    NAMES GL
    REQUIRED
  )

  target_compile_options(tigr PRIVATE
    -Wall
    -Wextra
    -pedantic
    -g
    -Og
  )

  target_link_libraries(tigr PUBLIC
    ${GLU}
    ${X11}
    ${GL}
  )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  find_package(OpenGL REQUIRED)

  target_link_libraries(tigr PUBLIC
    ${OPENGL_LIBRARIES}
    gdi32
  )

  target_compile_options(tigr PRIVATE
    -W4
  )
else()
  message("unsupported system")
endif()
