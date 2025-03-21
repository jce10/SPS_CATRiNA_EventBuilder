# Set the minimum required CMake version
cmake_minimum_required(VERSION 3.16)

# Use C++17 standard
set(CMAKE_CXX_STANDARD 17)

# Enable Position Independent Code (for shared libraries)
set(CMAKE_POSITION_INDEPENDENT_CODE On)

# Set default build type to Release if not explicitly set
if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_BUILD_TYPE "Release")
    message("Building release")  # Notify user of build mode
else()
    message("Building debug")
endif()

# Define project name
project(SPS_SABRE_EventBuilder)

# Find ROOT library (GUI component required)
find_package(ROOT REQUIRED COMPONENTS Gui)

# Define custom directories for output binaries and libraries
set(EVB_BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin)  # Executables go here
set(EVB_LIBRARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib) # Libraries go here

# Add dependencies and source directories
add_subdirectory(vendor/yaml-cpp)  # External YAML parser
add_subdirectory(src)               # Main source code