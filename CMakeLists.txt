cmake_minimum_required(VERSION 3.26)
set(PROJECT_NAME WebFrame)
set(CMAKE_CXX_STANDARD 20)
project(${PROJECT_NAME} LANGUAGES CXX)

# Attempt to set the vcpkg toolchain file only if CMAKE_TOOLCHAIN_FILE is not already defined
if (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(VCPKG_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
    
    if (EXISTS "${VCPKG_TOOLCHAIN_FILE}")
        set(CMAKE_TOOLCHAIN_FILE "${VCPKG_TOOLCHAIN_FILE}" CACHE STRING "Toolchain file for vcpkg integration")
    else()
        message(FATAL_ERROR
            "CMAKE_TOOLCHAIN_FILE is not set, and the expected vcpkg toolchain file was not found at:\n"
            "  ${VCPKG_TOOLCHAIN_FILE}\n"
            "Please set CMAKE_TOOLCHAIN_FILE manually, or ensure vcpkg is installed at:\n"
            "  ${CMAKE_SOURCE_DIR}/vcpkg"
        )
    endif()
endif()

set(VCPKG_PREFIX_PATH "${CMAKE_SOURCE_DIR}/vcpkg_installed/x64-windows" CACHE PATH "vcpkg install prefix")
if (NOT EXISTS "${VCPKG_PREFIX_PATH}")
    message(FATAL_ERROR "Vcpkg install prefix not found at ${VCPKG_PREFIX_PATH}"
            "\nLikely dependencies not installed. Install the dependencies first.")
endif()
set(CMAKE_PREFIX_PATH "${VCPKG_PREFIX_PATH}" CACHE PATH "cmake install prefix")

# Add executable target and source files
file(GLOB_RECURSE SOURCES src/*.cpp)
add_executable(${PROJECT_NAME} ${SOURCES})

find_path(SIMPLEINI_INCLUDE_DIRS "ConvertUTF.c")

# Include directories for all target sources
target_include_directories(${PROJECT_NAME} PRIVATE
    "src/window"
    "src/widgets"
    "src/webview"
    "src/utils"
    "src/helpers"
    ${Stb_INCLUDE_DIR}
    ${SIMPLEINI_INCLUDE_DIRS}
)

# Find Packages
find_package(imgui CONFIG REQUIRED)
find_package(unofficial-webview2 CONFIG REQUIRED)

# Manually link DirectX 11 & D3DCompiler
find_library(D3D11_LIB d3d11.lib)
find_library(D3DCOMPILER_LIB d3dcompiler.lib)

if(NOT D3D11_LIB)
    message(FATAL_ERROR "DirectX 11 library not found!")
endif()
if(NOT D3DCOMPILER_LIB)
    message(FATAL_ERROR "D3DCompiler library not found!")
endif()

# Link Libraries
target_link_libraries(${PROJECT_NAME} PRIVATE 
    imgui::imgui
    d3d11
    d3dcompiler
    unofficial::webview2::webview2
)

# Disable console window for release build
target_link_options(${PROJECT_NAME} PRIVATE 
    $<$<CONFIG:Release>:/SUBSYSTEM:windows>
    $<$<CONFIG:Release>:/ENTRY:mainCRTStartup>
)

# Define the VCPKG binary directory
set(VCPKG_BIN_DIR "${CMAKE_SOURCE_DIR}/vcpkg_installed/x64-windows/bin")
# Define the target output directory (where the executable is placed)
set(TARGET_BIN_DIR "${CMAKE_BINARY_DIR}${CMAKE_BUILD_TYPE}/$<CONFIG>")

# Copy all DLLs from VCPKG_BIN_DIR to TARGET_BIN_DIR
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${VCPKG_BIN_DIR} ${TARGET_BIN_DIR}
)

# Include the functions.cmake file
include(${CMAKE_SOURCE_DIR}/functions.cmake)

set(FILES
    "${CMAKE_SOURCE_DIR}/settings.ini"
)

set(SRC_DIRECTORIES
    "${CMAKE_SOURCE_DIR}/fonts"
    "${CMAKE_SOURCE_DIR}/icons"
)

# Copy files to the build directory during post build
copy_files_post_build(${PROJECT_NAME} "${FILES}" "${TARGET_BIN_DIR}")
# Copy directories to the build directory during post build
copy_dirs_post_build(${PROJECT_NAME} "${SRC_DIRECTORIES}" "${TARGET_BIN_DIR}")
