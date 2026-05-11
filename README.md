# OpenGL vs Vulkan

This project compares the OpenGL and Vulkan APIs in terms of API-related overhead and performance. The core idea is to create pairs of almost-identical tests in each API and benchmark them.

## Recent Modifications (vcpkg & Cleanup)

The project has been modernized with the following changes:
- **Dependency Management**: Migrated to `vcpkg`. No more manual submodule management or system dependency installation (except for drivers).
- **Code Cleanup**: Removed legacy `third_party`, `cmake`, and `scripts` directories.
- **Test Expansion**: All 4 tests now have both Simple and Multithreaded variants for both OpenGL and Vulkan (16 variants total).
- **Full Benchmarking**: The `-all` mode now executes all 16 variants and generates a comprehensive `results.csv`.

## Implemented Tests

| Test | GL Simple | GL Multi | VK Simple | VK Multi |
| :---: | :---: | :---: | :---: | :---: |
| Test #1 (Balls) | ✅ | ✅ | ✅ | ✅ |
| Test #2 (Terrain) | ✅ | ✅ | ✅ | ✅ |
| Test #3 (Shadows) | ✅ | ✅ | ✅ | ✅ |
| Test #4 (Init) | ✅ | ✅ | ✅ | ✅ |

## Multi-threading Implementation

### OpenGL: The Single-Threaded Reality
**OpenGL is inherently single-threaded.** It operates as a global machine state, and the graphics driver expects commands from a single thread. 
- In this project, "Multithreaded OpenGL" does **not** mean parallel graphics command submission.
- Instead, it refers to **paralellizing CPU-bound logic** (such as updating object positions or calculating LoD) across multiple cores using `std::thread`. The final submission to the GPU still happens on the main thread.

### Vulkan: Native Parallelism
**Vulkan is designed for multi-threading.** 
- It allows for **parallel Command Buffer recording** (using secondary command buffers) across all available CPU cores.
- It also supports parallel resource creation and asynchronous transfers.
- This allows Vulkan to significantly outperform OpenGL in CPU-bound scenarios by distributing the driver overhead across multiple cores.

## Building

### Requirements
- C++11 compiler
- CMake 3.10+
- **vcpkg** (Environment variable `VCPKG_ROOT` must be set)
- Vulkan SDK

### Build Instructions (Linux/Windows)

1. Set up vcpkg and install dependencies:
   ```bash
   vcpkg install glew glfw3 glm vulkan
   ```
2. Build using CMake:
   ```bash
   mkdir build && cd build
   cmake .. -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
   make -j$(nproc)
   ```

## Runtime Arguments

| Name | Argument type | Description |
| :--- | :---: | :---: |
| `-t` | integer | Specifies test number (1-4). |
| `-api` | string | Specifies API (`gl` or `vk`). |
| `-m` | - | Optional. Runs the multithreaded version. |
| `-all` | - | Runs all 16 test variants and generates `results.csv`. |
| `-benchmark` | - | Optional. Enables benchmarking mode. |
| `-time` | float | Optional. Changes benchmark duration (default: 15s). |

## Author
Original project by Damian Dyńdo (2017). 
Updated and expanded for TFG purposes (2026).
