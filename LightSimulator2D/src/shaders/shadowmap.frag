#version 430

uniform float u_MaskStrength;

layout(location = 0) out vec4 FinalColor;

void main()
{
	FinalColor = vec4(1.0, 1.0, 0.0, u_MaskStrength);
}