# Vulkan Rahmenprogramm Student Edition for masters course in computer graphics.


## Don't forget to get the submodules when cloning! So follow these steps!
Clone with
```bash
git clone --recurse-submodules https://gitlab.lrz.de/computergrafik-rahmenprogramme/vulkanrahmenprogrammstudentedition.git
```

or via SSH
```bash
git clone --recurse-submodules git@gitlab.lrz.de:computergrafik-rahmenprogramme/vulkanrahmenprogrammstudentedition.git
```

If you already cloned without --recurse-submodules, init the submodules with:
```bash
git submodule update --init --recursive
```

# Build using CMake

```cd``` into a newly created folder within this repos root-folder and do
```bash
cmake ..
```

This will build the default configuration for your system using GLFW as a windowing backend.

# NOTE

You need at least CMake version 3.24.x to generate the project files.
For the best developing experience on windows use the newest version
of Visual Studio (2022): https://discourse.cmake.org/t/cmake-3-24-does-not-set-the-include-path-for-visual-studio-correctly-when-using-vulkan/6598/3

VS 2022 introduced external includes. This - for example - prevents VS
to report warnings that come from an external library (and corresponding
header files) you don't have control over. The issue was that intellisense
didn't pick up those includes in an early version of VS2022.
When using Vulkan's cmake it will set the Vulkan SDK's include dir as
such an external include path in VS.

Cmake 3.24 allows to link easily against shaderc_compiler which allows
to compile GLSL to SPIR-V at runtime. shaderc_compiler comes with the
Vulkan SDK.
