cmake_minimum_required(VERSION 3.10)
project(3DSoft)

include(FetchContent)

FetchContent_Declare(SDL2
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG release-2.26.3
        )

FetchContent_MakeAvailable(SDL2)

include_directories(${CMAKE_PROJECT_NAME} ${sdl2_SOURCE_DIR}/include)

add_executable(${CMAKE_PROJECT_NAME})

target_sources(${CMAKE_PROJECT_NAME} PRIVATE main.c)
target_sources(${CMAKE_PROJECT_NAME} PRIVATE gMath.c)
target_sources(${CMAKE_PROJECT_NAME} PRIVATE gMath.h)
target_sources(${CMAKE_PROJECT_NAME} PRIVATE gRender.c)
target_sources(${CMAKE_PROJECT_NAME} PRIVATE gRender.h)

if (MSVC)
    target_link_libraries(${CMAKE_PROJECT_NAME} SDL2main SDL2)
else()
    target_link_libraries(${CMAKE_PROJECT_NAME} mingw32 SDL2main SDL2)
endif()

#DLLs
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        $<IF:$<TARGET_EXISTS:SDL2>,$<$<TARGET_EXISTS:SDL2>:$<TARGET_FILE:SDL2>>,"${PROJECT_SOURCE_DIR}/dummy">
        $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>)

