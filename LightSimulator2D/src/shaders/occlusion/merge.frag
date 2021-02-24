#version 430

in vec2 UV;

uniform float u_Weight;

layout(binding = 0) uniform sampler2D u_Sampler1;
layout(binding = 1) uniform sampler2D u_Sampler2;

layout(location = 0) out vec4 FinalColor;

void main()
{
	FinalColor = ((texture(u_Sampler1, UV) * u_Weight) + (texture(u_Sampler2, UV) * (1.0f - u_Weight))) / 2.0f;
}