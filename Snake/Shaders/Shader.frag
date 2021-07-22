#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 MVP;
	vec4 viewport;
    float radius;
    vec2 circleCenter;
} ubo;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in float fragIsCircle;
layout(location = 0) out vec4 outColor;




float scale(float pos, float oldMin, float oldMax, float newMin, float newMax) 
{
	float old_range = oldMax - oldMin;
	float new_range = newMax - newMin;
	return newMin + (pos - oldMin) * new_range / old_range;
}

vec2 scale(vec2 pos, float oldMin, float oldMax, float newMin, float newMax)
{
	return vec2(scale(pos.x, oldMin, oldMax, newMin, newMax), scale(pos.y, oldMin, oldMax, newMin, newMax));
}

void main() {	

	
	outColor = vec4(fragColor, 1.0);

	if(fragIsCircle != 0.0) 
	{
		// Position in window space;
		gl_FragCoord;

		// Position in [-1, 1];

		vec4 ndcPos;
		vec4 viewport = ubo.viewport;

		ndcPos.xy = ((2.0 * gl_FragCoord.xy) - (2.0 * viewport.xy)) / (viewport.zw) - 1;
		ndcPos.w = 1.0;

		vec4 clipPos = ndcPos;

		// circle center in [-1, 1]
		vec2 center = (ubo.MVP * vec4(ubo.circleCenter, 0.0, 1.0)).xy;


		float gridCount = 20;

		while((ubo.MVP * vec4(gridCount, 0, 0, 1)).x < 1) ++gridCount;
		

		float dist = sqrt((clipPos.x - center.x) * (clipPos.x - center.x) + (clipPos.y - center.y) * (clipPos.y - center.y));
		dist = scale(dist, 0, 2, 0, gridCount);

		if(dist > ubo.radius) discard;
	} 

}

