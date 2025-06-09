#version 450

#define MAX_VEL 5.0

struct Particle
{
    vec2 position;
    vec2 velocity;
};

layout (location = 0) out vec4 vertColor;

layout (set = 0, binding = 0) uniform Transform
{
    mat4 projection;
    vec4 staticColor;
    vec4 dynamicColor;
} ubo;

layout (set = 0, binding = 1) readonly buffer Data
{
    Particle vertices[];
} data;

void main()
{
    Particle vertex = data.vertices[gl_VertexIndex];
    float velocityMagnitude = length(vertex.velocity);
    float scale = length(vertex.velocity) / MAX_VEL;
   
    float intensity = smoothstep(0.0, 0.5 * MAX_VEL, velocityMagnitude);
    vertColor = mix(ubo.staticColor, ubo.dynamicColor, intensity);
    
    gl_Position = ubo.projection * vec4(vertex.position, 0.0, 1.0);
    gl_PointSize = 1.0;
}
