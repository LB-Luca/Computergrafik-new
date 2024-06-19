
cmake_minimum_required(VERSION 3.24)

project(VulkanRahmenprogrammStudi VERSION 1.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# NOTE: This is necessary so Assimp plays along nicely. A newer version of Assimp probably fixes this
#       but it has problems with CMake Install-Scripts on Windows. We should get rid off Assimp in
#       the future...
if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    message("MacOS System detected. Disable deprecation warnings...")
    if (CMAKE_COMPILER_IS_GNUCC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
    endif()
endif()



set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

find_package(Vulkan REQUIRED)
message("Cmake Vulkan Module Path: " ${Vulkan_INCLUDE_DIRS})
message("Cmake Vulkan Include Dir: " ${Vulkan_INCLUDE_DIR})

# INSTALL #########

#change the default installation path
message("${CMAKE_INSTALL_PREFIX}")

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    message("Install prefix set to: ${CMAKE_INSTALL_PREFIX}")
endif (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

set(CMAKE_INSTALL_PREFIX "${VulkanRahmenprogrammStudi_SOURCE_DIR}/ShipIt" CACHE PATH "VulkanRahmenprogrammStudi install prefix" FORCE)
message("${CMAKE_INSTALL_PREFIX}")

# Include the installation script
include(install_targets.cmake)

# Install assets (same path for all targets)
set(ASSETS_DIR "${CMAKE_SOURCE_DIR}/assets")
install(DIRECTORY ${ASSETS_DIR}
    DESTINATION ${CMAKE_INSTALL_PREFIX}
)

# ! INSTALL #########

add_subdirectory(dependencies/vkal)
add_subdirectory(dependencies/imageloader)
add_subdirectory(dependencies/tinyobjloader)


add_subdirectory(A1_Versuch1a)
add_subdirectory(A1_Versuch1c)
add_subdirectory(A3_Normalenvektoren)
add_subdirectory(A4_Normalenvektoren)
add_subdirectory(A4_Cubemapping)
add_subdirectory(A5_PCSS)
add_subdirectory(A5_Raytracing)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set_property(TARGET vkal            PROPERTY FOLDER "Libraries")
set_property(TARGET tinyobjloader   PROPERTY FOLDER "Libraries")

set_property(TARGET A1_Versuch1a        PROPERTY FOLDER "Aufgaben")
set_property(TARGET A1_Versuch1c        PROPERTY FOLDER "Aufgaben")
set_property(TARGET A3_Normalenvektoren PROPERTY FOLDER "Aufgaben")
set_property(TARGET A4_Normalenvektoren PROPERTY FOLDER "Aufgaben")
set_property(TARGET A4_Cubemapping      PROPERTY FOLDER "Aufgaben")
set_property(TARGET A5_PCSS             PROPERTY FOLDER "Aufgaben")
set_property(TARGET A5_Raytracing       PROPERTY FOLDER "Aufgaben")



