#!/bin/sh

glslc scene.vert -o scene_vert.spv
glslc scene.frag -o scene_frag.spv

glslc skybox.vert -o skybox_vert.spv
glslc skybox.frag -o skybox_frag.spv
