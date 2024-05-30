#version 450
#extension GL_ARB_separate_shader_objects : enable


layout(location = 0) out vec4 outColor;

layout (location = 0) in vec4 color;
layout (location = 1) in vec4 position_ndc;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec3 in_shading;
layout (location = 4) in mat4 view_matrix;

void main() 
{
	outColor = vec4(in_shading, 1.f);
}
