#version 450

layout(location = 0) out vec3 fragColor;

vec3 positions[6] = vec3[](
    vec3( -0.5,  0.4, 0.0),
    vec3(  0.5,  0.4, 0.0),
    vec3(    0, -0.47, 0.0),
    vec3( 0.25, -0.03, 0.0),
    vec3(-0.25, -0.03, 0.0),
    vec3( 0.0,  0.4, 0.0)
);

vec3 colors[6] = vec3[](
    vec3(1.0, 1.0, 0.39),
    vec3(1.0, 1.0, 0.39),
    vec3(1.0, 1.0, 0.39),
    vec3(0.0, 0.0, 0.0),
    vec3(0.0, 0.0, 0.0),
    vec3(0.0, 0.0, 0.0)
);

void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 1.0);
    fragColor = colors[gl_VertexIndex];
}

