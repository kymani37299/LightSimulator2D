#version 430

#include "common.h"

layout(location = 0) in vec4 in_Position;

out vec2 POS;

void main()
{
	POS = in_Position.xy;
	gl_Position = vec4(POS, 0.0, 1.0);
}