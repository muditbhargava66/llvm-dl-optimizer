cmake_minimum_required(VERSION 3.10)
project(llvm-dl-optimizer)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the LLVM_DIR variable
set(LLVM_DIR "/opt/homebrew/opt/llvm/lib/cmake/llvm")

# Find the LLVM package
find_package(LLVM REQUIRED CONFIG)

# Include LLVM headers
include_directories(${LLVM_INCLUDE_DIRS})

# Link against LLVM libraries
link_directories(${LLVM_LIBRARY_DIRS})

# Add the include directory
include_directories(include)

# Add the source files
file(GLOB_RECURSE SOURCES "lib/*.cpp")

# Add the executable
add_executable(llvm-dl-optimizer ${SOURCES})

# Link against LLVM libraries
target_link_libraries(llvm-dl-optimizer ${LLVM_LIBRARIES})

# Enable RTTI
set_target_properties(llvm-dl-optimizer PROPERTIES
                      COMPILE_FLAGS "-fno-rtti"
                      LINK_FLAGS "-fno-rtti")

# Add the test directory
# add_subdirectory(tests)

# Add the examples directory
# add_subdirectory(examples)