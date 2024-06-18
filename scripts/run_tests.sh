#!/bin/bash

# Set the build directory
BUILD_DIR="build"

# Change to the build directory
cd $BUILD_DIR

# Run the tests
ctest --output-on-failure

# Return to the previous directory
cd ..