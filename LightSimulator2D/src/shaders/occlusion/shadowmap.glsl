#start VERTEX

#include "common.h"

layout(location = 0) in vec4 in_Position;

out vec2 POS;

void main()
{
	POS = in_Position.xy;
	gl_Position = vec4(POS, 0.0, 1.0);
}

#start FRAGMENT

in vec2 POS;

uniform float u_MaskStrength;
uniform vec2 u_LightPos;
uniform vec3 u_LightColor;

layout(location = 0) out vec4 FinalColor;

const float lightRadius = 1.2;

void main()
{
	float d = 10.0 * length(POS - u_LightPos);
	d *= 10.0f;
	d = lightRadius / (1.0 + 0.1 * d + 0.01 * d * d);
	FinalColor = vec4(u_LightColor, u_MaskStrength*d);
}