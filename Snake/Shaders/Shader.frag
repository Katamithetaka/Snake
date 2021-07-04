#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 0) out vec4 outColor;

vec2 circleCenter = vec2(0.0, 0.0);
float circleRadius = 0.4;
float circleBorder = 0.01;
vec2 viewPosition = vec2(400.0, 400.0);
vec2 screenCenter = viewPosition / 2;

void main() {	
	outColor = vec4(fragColor, 1.0);
	outColor = vec4(1.0);
}

