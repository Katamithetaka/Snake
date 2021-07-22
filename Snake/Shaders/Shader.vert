#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 MVP;
	vec4 viewport;
    float radius;
    vec2 circleCenter;
} ubo;

layout(location = 0) in vec2 pos;
layout(location = 1) in vec3 color;
layout(location = 2) in float isCircle;

vec2 positions[4] = vec2[](
    vec2(0, 0),
    vec2(0, 20),
    vec2(20, 20),
    vec2(20, 0)
);


layout(location = 0) out vec3 fragColor;
layout(location = 1) out float fragIsCircle;

void main() {
    gl_Position = ubo.MVP * vec4(pos.xy, 0.0, 1.0);
    fragColor = color;
    fragIsCircle = isCircle;
}

