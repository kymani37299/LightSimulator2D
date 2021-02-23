#version 430

in vec2 UV;

layout(binding = 0) uniform sampler2D u_Albedo;
layout(binding = 1) uniform sampler2D u_OcclusionMask1;
layout(binding = 2) uniform sampler2D u_OcclusionMask2;

layout(location = 0) out vec4 FinalColor;

const float baseLight = 0.3;

void main()
{
	vec3 occlusion = (texture(u_OcclusionMask1, UV).rgb + texture(u_OcclusionMask2, UV).rgb) / 2.0f;
	vec3 lightMask = baseLight + occlusion;
	lightMask = clamp(lightMask,0.0,1.0);

	vec3 col = texture(u_Albedo, UV).rgb*0.7;
	col += 0.3;
	col *= lightMask;

	FinalColor = vec4(col, 1.0);
}