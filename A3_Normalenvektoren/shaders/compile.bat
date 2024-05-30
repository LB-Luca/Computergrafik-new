@echo off

%VULKAN_SDK%/bin/glslc gouraud_shader.vert -o gouraud_vert.spv
%VULKAN_SDK%/bin/glslc gouraud_shader.frag -o gouraud_frag.spv
