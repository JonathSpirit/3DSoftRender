cmake_minimum_required(VERSION 3.23)
project(3DSoft)

include(FetchContent)

set(FETCHCONTENT_TRY_FIND_PACKAGE_MODE ALWAYS)

FetchContent_Declare(SDL2
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG release-2.24.0
        )

FetchContent_Declare(SDL2_ttf
        GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf
        GIT_TAG main
        )

FetchContent_MakeAvailable(SDL2 SDL2_ttf)

include_directories(${CMAKE_PROJECT_NAME} ${SDL2_INCLUDE_DIR})

add_executable(${CMAKE_PROJECT_NAME})

target_sources(${CMAKE_PROJECT_NAME} PRIVATE main.c)

target_link_libraries(${CMAKE_PROJECT_NAME} mingw32 SDL2main SDL2 SDL2_ttf)

#DLLs
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        $<IF:$<TARGET_EXISTS:SDL2>,$<$<TARGET_EXISTS:SDL2>:$<TARGET_FILE:SDL2>>,"${PROJECT_SOURCE_DIR}/dummy">
        $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>)

add_custom_command(TARGET ${CMAKE_PROJECT_NAME} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        $<IF:$<TARGET_EXISTS:SDL2_ttf>,$<$<TARGET_EXISTS:SDL2_ttf>:$<TARGET_FILE:SDL2_ttf>>,"${PROJECT_SOURCE_DIR}/dummy">
        $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>)
