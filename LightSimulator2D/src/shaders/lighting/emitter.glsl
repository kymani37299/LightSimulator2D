#start VERTEX

void main() { }

#start GEOMETRY

#include "point2quad.h"

#start FRAGMENT

in vec2 POS;
in vec2 UV;

layout(binding = 0) uniform sampler2D u_Texture;

uniform vec3 u_EmitterColor;

layout(location = 0) out vec4 FinalColor;

const float alphaTreshold = 0.01;

void main()
{
	vec4 tex = texture(u_Texture, UV);
	if (tex.a < alphaTreshold) discard;
	FinalColor = vec4(u_EmitterColor, 1.0);
}