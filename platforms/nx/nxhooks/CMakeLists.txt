cmake_minimum_required(VERSION 2.6)

project(nxhooks)

cmake_policy(SET CMP0004 OLD)
find_package(SDL2 2 REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/include ${SDL2_INCLUDE_DIRS})

add_library(dl STATIC src/dlstub.c)

set(NXHOOKS_SOURCES src/init.c src/sdl/cursor.c src/sdl/input.c)
if(NINTENDO_SWITCH2)
    list(APPEND NXHOOKS_SOURCES src/nmouse.c src/touch.c)
endif()
add_library(nxhooks STATIC ${NXHOOKS_SOURCES})

target_link_libraries(dl nxhooks glad EGL glapi drm_nouveau mpg123 modplug vorbisidec ogg nx)
