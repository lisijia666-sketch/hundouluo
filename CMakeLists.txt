cmake_minimum_required(VERSION 3.16)
project(ArcadeRunGun LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(arcade_run_gun src/main.cpp)
target_compile_definitions(arcade_run_gun PRIVATE SDL_MAIN_HANDLED)

if(MINGW)
    get_filename_component(MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
    foreach(runtime_dll libgcc_s_seh-1.dll libstdc++-6.dll libwinpthread-1.dll SDL2.dll)
        find_file(${runtime_dll}_PATH
            NAMES ${runtime_dll}
            PATHS "${MINGW_BIN_DIR}"
            NO_DEFAULT_PATH)
        if(${runtime_dll}_PATH)
            add_custom_command(TARGET arcade_run_gun POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${${runtime_dll}_PATH}"
                    "$<TARGET_FILE_DIR:arcade_run_gun>")
        endif()
    endforeach()
endif()

if(MSVC)
    target_compile_options(arcade_run_gun PRIVATE /W4 /permissive-)
else()
    target_compile_options(arcade_run_gun PRIVATE -Wall -Wextra -Wpedantic)
endif()

find_package(SDL2 CONFIG QUIET)

if(TARGET SDL2::SDL2)
    target_link_libraries(arcade_run_gun PRIVATE SDL2::SDL2)
else()
    find_package(SDL2 REQUIRED)
    target_include_directories(arcade_run_gun PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(arcade_run_gun PRIVATE ${SDL2_LIBRARIES})
endif()
