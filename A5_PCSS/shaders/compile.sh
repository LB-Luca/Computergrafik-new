#!/bin/sh

glslc shader.vert -o vert.spv
glslc shader.frag -o frag.spv

glslc shadowmap.vert -o shadowmap_vert.spv
glslc shadowmap.frag -o shadowmap_frag.spv
