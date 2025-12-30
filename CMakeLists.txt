cmake_minimum_required (VERSION 3.20)
project("CyEcho" LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# --- libuiohook ---
FetchContent_Declare(
    libuiohook 
    GIT_REPOSITORY https://github.com/kwhat/libuiohook 
    GIT_TAG 1.2.2
    GIT_SHALLOW TRUE
)

# --- ImGui ---
FetchContent_Declare(
    imgui 
    GIT_REPOSITORY https://github.com/ocornut/imgui.git 
    GIT_TAG docking
    GIT_SHALLOW TRUE
)

# Global settings for dependencies
set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
set(BUILD_DEMOS OFF CACHE INTERNAL "")
set(USE_X11 ON CACHE INTERNAL "")

FetchContent_MakeAvailable(libuiohook imgui)
file(GLOB IMGUI_SOURCES "${imgui_SOURCE_DIR}/*.cpp")
add_library(imgui STATIC ${IMGUI_SOURCES})
set(IMGUI_BACKENDS
    "${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp"
    "${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp"
)

file(GLOB_RECURSE APP_SOURCES 
    "src/*.cpp" 
    "src/*.h"
    "src/*.hpp"
    "src/*.c"
)

if(WIN32)
    set(RES_FILE "resources.rc")
endif()

# --- Executable Target ---
# WIN32 flag hides the console and expects WinMain
add_executable(CyEcho WIN32 ${APP_SOURCES} ${IMGUI_BACKENDS} ${RES_FILE})

# --- Include Directories ---
target_include_directories(CyEcho PRIVATE 
    src 
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

# --- Linking ---
target_link_libraries(CyEcho PRIVATE 
    uiohook
    imgui
    d3d11
    d3dcompiler
    dwmapi
    comctl32
)

# --- Resource Handling ---
set(RES_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources")
set(RES_TARGET_DIR "${CMAKE_CURRENT_BINARY_DIR}/resources")

# Ensure directory exists and copy
add_custom_command(TARGET CyEcho POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${RES_SOURCE_DIR}"
        "${RES_TARGET_DIR}"
    COMMENT "Copying resources to build directory..."
)

# OS Specific requirements
if(APPLE)
    target_link_libraries(CyEcho PRIVATE 
        "-framework Cocoa"
        "-framework IOKit"
        "-framework AppKit"
    )
endif()