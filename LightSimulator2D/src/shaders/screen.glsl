#start VERTEX

layout(location = 0) in vec2 in_Position;
layout(location = 1) in vec2 in_UV;

out vec2 UV;

void main()
{
	UV = in_UV;
	gl_Position = vec4(in_Position,0.0,1.0);
}

#start FRAGMENT

in vec2 UV;

layout(binding = 0) uniform sampler2D u_Screen;

layout(location = 0) out vec4 FinalColor;

void main()
{
	FinalColor = texture(u_Screen,UV);
}