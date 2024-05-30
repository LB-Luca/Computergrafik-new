@echo off

%VULKAN_SDK%/bin/glslc shader.vert -o vert.spv
%VULKAN_SDK%/bin/glslc shader.frag -o frag.spv

%VULKAN_SDK%/bin/glslc shadowmap.vert -o shadowmap_vert.spv
%VULKAN_SDK%/bin/glslc shadowmap.frag -o shadowmap_frag.spv