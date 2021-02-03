#version 330

in vec2 UV;

uniform sampler2D u_Texture;

layout(location = 0) out vec4 FinalColor;

void main()
{
	FinalColor = texture(u_Texture, UV);
}