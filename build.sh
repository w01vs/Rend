#!/bin/bash

# --- CHECK NINJA AVAILABILITY ---
if ! command -v ninja &> /dev/null
then
    echo "❌ Error: Ninja build system is required but was not found in your PATH." >&2
    echo "Please install Ninja to continue (e.g., 'sudo apt install ninja-build' or 'brew install ninja')." >&2
    exit 1
fi

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
    # The -G Ninja flag is now mandatory.
    echo "Configuring with: cmake -S $SOURCE_DIR -B $BUILD_DIR -G Ninja -DCMAKE_BUILD_TYPE=$BUILD_TYPE"
    cmake -S "$SOURCE_DIR" -B "$BUILD_DIR" -G Ninja -DCMAKE_BUILD_TYPE="$BUILD_TYPE"

    # Check if CMake configuration failed
    if [ $? -ne 0 ]; then
        echo "❌ CMake configuration failed for $BUILD_TYPE." >&2
        exit 1
    fi

    # 3. Build the project
    # cmake --build automatically uses the generator specified during configuration.
    cmake --build "$BUILD_DIR"
}

# --- EXECUTION ---

# Build the Debug configuration
build_config Debug

# Build the Release configuration
# build_config Release