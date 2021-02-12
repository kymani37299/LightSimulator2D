#version 330

in vec2 UV;

uniform sampler2D u_Texture;

layout(location = 0) out vec4 FinalColor;

const float alphaTreshold = 0.01;

void main()
{
	vec4 tex = texture(u_Texture, UV);
	//if (tex.a < alphaTreshold) discard;
	FinalColor = tex;
}