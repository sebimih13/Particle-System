#version 450

#extension GL_ARB_separate_shader_objects : enable // TODO: delete?
// #extension GL_KHR_vulkan_glsl : enable // TODO: delete? optional

#define MAX_VEL 5.0

struct Particle
{
    vec2 position;
    vec2 velocity;
    vec4 color;
};

layout (location = 0) out vec4 vertColor;

layout (set = 0, binding = 0) uniform Transform
{
    mat4 projection;
} ubo;

layout (set = 0, binding = 1) readonly buffer Data
{
    Particle vertices[];
} data;

void main()
{
    Particle vertex = data.vertices[gl_VertexIndex];
    float scale = length(vertex.velocity) / MAX_VEL;
    float inv = 1.0 - scale;
    vertColor = vec4(inv / 4.0, inv / 3.0, scale, 0.1);
    gl_Position = ubo.projection * vec4(vertex.position, 0.0, 1.0);
    gl_PointSize = 1.0;
}
