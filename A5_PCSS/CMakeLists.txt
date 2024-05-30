cmake_minimum_required(VERSION 3.24)
project(A5_PCSS VERSION 1.0)


file(GLOB_RECURSE SRC_FILES LIST_DIRECTORIES false RELATIVE
     ${CMAKE_CURRENT_SOURCE_DIR} *.c??)
file(GLOB_RECURSE HEADER_FILES LIST_DIRECTORIES false RELATIVE
     ${CMAKE_CURRENT_SOURCE_DIR} *.h)   

add_executable(A5_PCSS
    ${SRC_FILES}
    ${HEADER_FILES}#
    shaders/shadowmap.vert
    shaders/shadowmap.frag
    shaders/shader.vert
    shaders/shader.frag
    ../dependencies/utils/model.cpp
    ../dependencies/utils/model.h
    ../dependencies/utils/camera.cpp
    ../dependencies/utils/camera.h
    ../dependencies/stb/stb_image.h
    ../dependencies/vkal/src/examples/utils/platform.cpp
    ../dependencies/vkal/src/examples/utils/platform.h
    ../dependencies/vkal/src/examples/utils/glslcompile.cpp
    ../dependencies/vkal/src/examples/utils/glslcompile.h
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

target_include_directories(A5_PCSS
    PUBLIC ../dependencies
    PUBLIC ../dependencies/imgui
    PUBLIC ${VULKAN_INCLUDE_DIR}
)

target_link_libraries(A5_PCSS
    PUBLIC vkal
    PUBLIC tinyobjloader
)


set_property(TARGET A5_PCSS PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/A5_PCSS")

# Install this target
install_target(${PROJECT_NAME})
