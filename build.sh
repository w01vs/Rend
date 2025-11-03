#!/bin/bash

# The source directory is the current directory (where CMakeLists.txt is)
SOURCE_DIR=$(pwd)

# Function to build a specific configuration in its own directory
build_config() {
    local BUILD_TYPE=$1
    local BUILD_DIR="build-${BUILD_TYPE}"

    echo "--- Configuring and building $BUILD_TYPE in $BUILD_DIR ---"

    # 1. Create the build directory if it doesn't exist
    mkdir -p "$BUILD_DIR"

    # 2. Run CMake (Configure)
    cmake -S "$SOURCE_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"

    # 3. Build the project
    cmake --build "$BUILD_DIR"
}

# Build the Debug configuration
build_config Debug

# Build the Release configuration
# build_config Release