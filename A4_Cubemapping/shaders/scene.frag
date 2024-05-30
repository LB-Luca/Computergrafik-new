#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform UBO
{
    mat4 view;
    mat4 projection;
    uint texture_mapping_type;
};
//Sampler für die Cube Map
layout(set = 0, binding = 1) uniform samplerCube cubemap_sampler;

layout (location = 1) in vec4 viewspace_pos;
layout (location = 2) in vec4 viewspace_normal;
layout (location = 3) in vec4 position_ndc;
layout (location = 4) in vec3 texCoords;
layout (location = 5) in mat4 view_matrix;
layout (location = 9) in mat4 model_matrix;

void main() 
{   
	
	vec4 color = vec4(0);

	if (texture_mapping_type == 0) { // OBJECT LINEAR
		
		color = vec4(1,0,0,1);	
	}
	else if (texture_mapping_type == 1) {// EYE LINEAR

	    color = vec4(0,1,0,1);
	}
	else { // Using Cubemap for reflection

		color = vec4(0,0,1,1);
	}

    outColor = vec4(color.xyz, 1.f);
}