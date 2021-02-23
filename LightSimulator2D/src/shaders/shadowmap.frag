#version 430

layout(location = 0) in vec2 u_Position;

uniform float u_MaskStrength;
uniform vec2 u_LightPos;

layout(location = 0) out vec4 FinalColor;

const float lightRadius = 1.2;

void main()
{
	float d = length(u_Position - u_LightPos);
	d *= 10.0f;
	d = lightRadius / (1.0 + 0.1 * d + 0.01 * d * d);
	FinalColor = vec4(1.0, 1.0, 0.0, u_MaskStrength*d);
}