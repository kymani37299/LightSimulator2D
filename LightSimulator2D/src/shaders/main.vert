#version 430

layout(location = 0) in vec2 in_Position;
layout(location = 1) in vec2 in_UV;

uniform mat3 u_Transform;

out vec2 UV;

void main()
{
	vec3 pos = vec3(in_Position, 1.0) * u_Transform;
	gl_Position = vec4(pos.xy,0.0,1.0);
	UV = in_UV;
}