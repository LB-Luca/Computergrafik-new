#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 in_uv;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform PushConstant {
    uint texture_index;
} MaterialPC;

layout(set = 1, binding = 0) uniform sampler2D textures[];

void main() {
    vec4 texture_color = texture(textures[MaterialPC.texture_index], in_uv);
    //outColor = vec4(0.5 * normal + 0.5, 1.0);
    outColor = vec4(texture_color.rgb, 1.0);
}