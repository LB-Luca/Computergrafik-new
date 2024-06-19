@echo off

%VULKAN_SDK%/bin/glslc gouraud_shader.vert -o gouraud_vert.spv
%VULKAN_SDK%/bin/glslc gouraud_shader.frag -o gouraud_frag.spv

%VULKAN_SDK%/bin/glslc phong_shader.vert -o phong_vert.spv
%VULKAN_SDK%/bin/glslc phong_shader.frag -o phong_frag.spv