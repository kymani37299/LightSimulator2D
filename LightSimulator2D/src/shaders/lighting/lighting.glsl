#start VERTEX

layout(location = 0) in vec2 in_Position;
layout(location = 1) in vec2 in_UV;

out vec2 UV;

void main()
{
	gl_Position = vec4(in_Position, 0.0, 1.0);
	UV = in_UV;
}

#start FRAGMENT

in vec2 UV;

layout(binding = 0) uniform sampler2D u_Albedo;
layout(binding = 1) uniform sampler2D u_OcclusionMask;

uniform vec3 u_AmbientLight;

layout(location = 0) out vec4 FinalColor;

const float baseLight = 0.1;

void main()
{
	vec3 occlusion = texture(u_OcclusionMask, UV).rgb;
	vec3 lightMask = u_AmbientLight + occlusion;
	lightMask = clamp(lightMask,0.0,1.0);

	vec3 col = texture(u_Albedo, UV).rgb;
	col = clamp(col,0.0,1.0);
	col *= lightMask;

	FinalColor = vec4(col, 1.0);
}