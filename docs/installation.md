# Installation Guide

## Prerequisites
Before installing the LLVM-based Deep Learning Optimizer, ensure that you have the following prerequisites installed on your system:
- LLVM (version 10.0 or higher)
  - For macOS with Apple Silicon, you may need to build LLVM from source or use a compatible binary distribution.
- CMake (version 3.10 or higher)
- C++ compiler with C++17 support (e.g., GCC, Clang, Apple Clang)

## Installation Steps
Follow these steps to install the optimizer:

1. Clone the repository:
   ```
   git clone https://github.com/muditbhargava66/llvm-dl-optimizer.git
   ```

2. Change to the project directory:
   ```
   cd llvm-dl-optimizer
   ```

3. Create a build directory and navigate to it:
   ```
   mkdir build
   cd build
   ```

4. Configure the project using CMake:
   ```
   cmake ..
   ```

5. Build the project:
   ```
   make
   ```

6. (Optional) Run the tests to verify the installation:
   ```
   ctest
   ```

7. (Optional) Install the optimizer:
   ```
   make install
   ```

## Troubleshooting
If you encounter any issues during the installation process, consider the following:
- Ensure that you have the required prerequisites installed and properly configured.
- Verify that you have the necessary permissions to clone the repository and create directories.
- Check that you have a compatible version of LLVM installed and the `LLVM_DIR` environment variable is set correctly.
- If the build fails, examine the error messages and consult the project's issue tracker for known issues and solutions.

If you still face problems, please open an issue on the project's GitHub repository, providing detailed information about your setup and the encountered error.

---