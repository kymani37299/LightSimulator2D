#start VERTEX

void main() { }

#start GEOMETRY

#include "point2quad.h"

#start FRAGMENT

in vec2 POS;
in vec2 UV;

uniform vec3 u_Color;

layout(location = 0) out vec4 FinalColor;

void main()
{
	FinalColor = vec4(u_Color,1.0f);
}