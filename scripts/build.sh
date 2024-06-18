#!/bin/bash

# Set the build directory
BUILD_DIR="build"

# Create the build directory if it doesn't exist
mkdir -p $BUILD_DIR

# Change to the build directory
cd $BUILD_DIR

# Configure the project with CMake
cmake ..

# Build the project
make

# Return to the previous directory
cd ..