cmake_minimum_required (VERSION 3.20)

project("CyEcho" LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- FetchContent for libuiohook ---
include(FetchContent)
FetchContent_Declare(
    libuiohook 
    GIT_REPOSITORY https://github.com/kwhat/libuiohook 
    GIT_TAG 1.2.2
)
FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI
  GIT_TAG v6.1.9
)
 

# libuiohook specific options: Build it as a static library to avoid DLL issues
set(USE_X11 ON CACHE INTERNAL "")
set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
set(BUILD_DEMOS OFF CACHE INTERNAL "")
set(RES_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources")
set(RES_TARGET_DIR "${CMAKE_CURRENT_BINARY_DIR}/resources")

FetchContent_MakeAvailable(libuiohook)
FetchContent_MakeAvailable(ftxui)
# --- Source Files ---
# Note: It's better to list files or use a specific directory 
# than GLOB, but for small projects, this works.
file(GLOB_RECURSE APP_SOURCES 
    "src/*.cpp" 
    "src/*.h"
    "src/*.c"
)

# --- Executable Target ---
add_executable(CyEcho ${APP_SOURCES})


target_include_directories(CyEcho PRIVATE src)

target_link_libraries(CyEcho PRIVATE 
    uiohook
      PRIVATE ftxui::screen
      PRIVATE ftxui::dom
      PRIVATE ftxui::component
)

file(COPY "${RES_SOURCE_DIR}" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")


add_custom_command(TARGET CyEcho POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${RES_SOURCE_DIR}"
        "${RES_TARGET_DIR}"
    COMMENT "Copying resources to build directory..."
)

# OS Specific requirements for libuiohook
if(APPLE)
    target_link_libraries(CyEcho PRIVATE 
        "-framework Cocoa"
        "-framework IOKit"
        "-framework AppKit"
    )
elseif(WIN32)
    target_link_libraries(CyEcho PRIVATE comctl32)
endif()