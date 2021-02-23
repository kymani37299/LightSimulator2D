#version 430

layout(location = 0) in vec2 u_Position;

uniform float u_MaskStrength;
uniform vec2 u_LightPos;

layout(location = 0) out vec4 FinalColor;

const float lightRadius = 1.5;

void main()
{
	float d = lightRadius - length(u_Position - u_LightPos);
	FinalColor = vec4(1.0, 1.0, 0.0, u_MaskStrength*d);
}