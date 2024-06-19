#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec4 color;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec3 world_position;
layout (location = 4) in vec3 world_normal;

layout(set = 0, binding = 1) uniform LightUBO
{
    vec4 light_pos;
    vec4 light_ambient;
    vec4 light_diffuse;
    vec4 light_specular;
    uint show_normals;
};

layout (set = 0, binding = 2) uniform MaterialUBO
{
    vec4 emissive;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
} Material;

layout (set = 0, binding = 0) uniform UBO
{
    mat4 view;
    mat4 projection;
};

layout(location = 0) out vec4 outColor;

void main() {
    vec3 light_vector = normalize(light_pos.xyz - world_position);
    // vec3 eye_view_position = (inverse(view) * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
    vec3 view_vector = (normalize(-world_position)).xyz;
    vec3 reflect_vector = reflect(light_vector, world_normal);

    float normal_light_dot = max(dot(world_normal, light_vector), 0.0);
    float view_light_dot = clamp(dot(reflect_vector, view_vector), 0.0, 1.0);

    vec3 emissive = Material.emissive.rgb;

    vec3 ambient = Material.ambient.rgb * light_ambient.rgb;

    vec3 diffuse = normal_light_dot * Material.diffuse.rgb * light_diffuse.rgb;

    float specular_intensity = pow(view_light_dot, Material.specular.w);
    vec3 specular = specular_intensity * Material.specular.rgb * light_specular.rgb;

    outColor = vec4(emissive + ambient + diffuse + specular, 1.0);
}