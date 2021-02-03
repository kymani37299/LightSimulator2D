#version 330

in vec2 UV;

layout(location = 0) out vec4 FinalColor;

void main()
{
	FinalColor = vec4(UV, 0.0, 1.0);
}