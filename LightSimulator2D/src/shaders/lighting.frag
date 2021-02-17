#version 430

in vec2 UV;

layout(binding = 0) uniform sampler2D u_Albedo;
layout(binding = 1) uniform sampler2D u_OcclusionMask;

layout(location = 0) out vec4 FinalColor;

const float baseLight = 0.3;

void main()
{
	vec3 lightMask = baseLight + texture(u_OcclusionMask, UV).rgb;
	lightMask = clamp(lightMask,0.0,1.0);

	vec3 col = texture(u_Albedo, UV).rgb;
	col *= lightMask;

	FinalColor = vec4(col, 1.0);
}