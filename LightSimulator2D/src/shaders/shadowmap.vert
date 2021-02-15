#version 430

#include "common.h"

#ifdef GPU_OCCLUSION

layout(location = 0) in vec4 in_Position;

void main()
{
	gl_Position = vec4(in_Position.xy, 0.0, 1.0);
}


#else

layout(location = 0) in vec2 in_Position;

void main()
{
	gl_Position = vec4(in_Position, 0.0, 1.0);
}

#endif