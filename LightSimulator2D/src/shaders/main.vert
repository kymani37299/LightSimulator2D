#version 330

#define MAX_BATCH_SIZE 100

layout(location = 0) in vec2 in_Position;
layout(location = 1) in vec2 in_UV;

layout(std140) uniform StaticBuffer 
{
	mat3 u_StaticTransform[MAX_BATCH_SIZE];
};

uniform int u_StaticIndex;
uniform mat3 u_Transform;

out vec2 UV;

void main()
{
	mat3 transform = u_StaticIndex == -1 ? u_Transform : u_StaticTransform[u_StaticIndex];
	vec3 pos = vec3(in_Position, 1.0) * transform;
	gl_Position = vec4(pos.xy,0.0,1.0);
	UV = in_UV;
}