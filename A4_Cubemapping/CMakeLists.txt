cmake_minimum_required(VERSION 3.24)
project(A4_Cubemapping VERSION 1.0)


file(GLOB_RECURSE SRC_FILES LIST_DIRECTORIES false RELATIVE
     ${CMAKE_CURRENT_SOURCE_DIR} *.c??)
file(GLOB_RECURSE HEADER_FILES LIST_DIRECTORIES false RELATIVE
     ${CMAKE_CURRENT_SOURCE_DIR} *.h)   

add_executable(A4_Cubemapping
    ${SRC_FILES}
    ${HEADER_FILES}
    shaders/scene.vert
    shaders/scene.frag
    shaders/skybox.vert
    shaders/skybox.frag
    ../dependencies/utils/model.cpp
    ../dependencies/utils/model.h
    ../dependencies/utils/camera.cpp
    ../dependencies/utils/camera.h
    ../dependencies/stb/stb_image.h
    ../dependencies/vkal/src/examples/utils/platform.cpp
    ../dependencies/vkal/src/examples/utils/platform.h
    ../dependencies/modelloader/Model.cpp
    ../dependencies/modelloader/Model.h
    ../dependencies/imgui/backends/imgui_impl_glfw.cpp
    ../dependencies/imgui/backends/imgui_impl_glfw.h
    ../dependencies/imgui/backends/imgui_impl_vulkan.cpp
    ../dependencies/imgui/backends/imgui_impl_vulkan.h
    ../dependencies/imgui/imgui.cpp
    ../dependencies/imgui/imgui.h
    ../dependencies/imgui/imgui_draw.cpp
    ../dependencies/imgui/imgui_tables.cpp
    ../dependencies/imgui/imgui_widgets.cpp
    ../dependencies/utils/imgui_helper.cpp
    ../dependencies/utils/imgui_helper.h
)

target_include_directories(A4_Cubemapping
    PUBLIC ../dependencies
    PUBLIC ../dependencies/imgui
    PUBLIC ${VULKAN_INCLUDE_DIR}
)

target_link_libraries(A4_Cubemapping
    PUBLIC vkal
    PUBLIC tinyobjloader
)


set_property(TARGET A4_Cubemapping PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/A4_Cubemapping")

# Install this target
install_target(${PROJECT_NAME})