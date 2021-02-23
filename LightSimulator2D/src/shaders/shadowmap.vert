#version 430

#include "common.h"

layout(location = 0) in vec4 in_Position;

layout(location = 0) out vec2 u_Position;

void main()
{
	u_Position = in_Position.xy;
	gl_Position = vec4(u_Position, 0.0, 1.0);
}