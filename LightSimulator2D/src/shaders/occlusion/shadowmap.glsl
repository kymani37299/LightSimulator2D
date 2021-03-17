#start VERTEX

layout(location = 0) in vec4 in_Position;

out vec2 POS;

void main()
{
	POS = in_Position.xy;
	gl_Position = vec4(POS, 0.0, 1.0);
}

#start FRAGMENT

#include "attenuation.h"

in vec2 POS;

uniform float u_MaskStrength;
uniform vec2 u_LightPos;
uniform vec3 u_LightColor;
uniform float u_LightRadius;

layout(location = 0) out vec4 FinalColor;

void main()
{
	float d = length(POS - u_LightPos);
	float attenuation = u_LightRadius*Attenuate(d);
	FinalColor = vec4(u_LightColor, u_MaskStrength*attenuation);
}