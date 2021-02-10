#version 330

layout(location = 0) in vec2 in_Position;

void main()
{
	vec3 pos = vec3(in_Position, 1.0);
	gl_Position = vec4(pos.xy, 0.0, 1.0);
}