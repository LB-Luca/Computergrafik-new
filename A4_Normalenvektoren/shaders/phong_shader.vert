#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec4 color;
layout (location = 4) in vec3 tangent;

layout (set = 0, binding = 0) uniform UBO
{
    mat4 view;
    mat4 projection;
};

// ...

layout (set = 0, binding = 3) uniform ModelUBO
{
    mat4 model_mat;
} Model;

layout (location = 0) out vec4 color_out;
// ...
layout (location = 2) out vec2 uv_out;
layout (location = 3) out vec3 world_position;
layout (location = 4) out vec3 world_normal;

void main() {
    gl_Position = projection * view * Model.model_mat * vec4(position, 1.0);
    color_out = color;
    uv_out = uv;
    world_position = (Model.model_mat * vec4(position, 1.0)).xyz;
    world_normal = normalize(Model.model_mat * vec4(normal, 0.0)).xyz;
}