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
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Link against LLVM libraries
link_directories(${LLVM_LIBRARY_DIRS})

# Add the include directory
include_directories(include)

# Add the source files
file(GLOB_RECURSE SOURCES "lib/*.cpp")
list(APPEND SOURCES "src/main.cpp")  # Ensure the main function file is included
message(STATUS "Source files: ${SOURCES}")

# Add the executable
add_executable(llvm-dl-optimizer ${SOURCES})

# Link against LLVM libraries
llvm_map_components_to_libnames(llvm_libs support core irreader passes)
target_link_libraries(llvm-dl-optimizer ${llvm_libs})

# Enable RTTI
set_target_properties(llvm-dl-optimizer PROPERTIES
                      COMPILE_FLAGS "-frtti"
                      LINK_FLAGS "-frtti")

# Add the test directory (uncomment if needed)
# add_subdirectory(tests)

# Add the examples directory (uncomment if needed)
# add_subdirectory(examples)
