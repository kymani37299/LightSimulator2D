#start VERTEX

void main() { }

#start GEOMETRY

#include "point2triangle.h"

#start FRAGMENT

in vec2 POS;
in vec2 UV;

layout(binding = 0) uniform sampler2D u_Texture;

layout(location = 0) out vec4 FinalColor;

const float alphaTreshold = 0.01;

void main()
{
	vec4 tex = texture(u_Texture, UV);
	if (tex.a < alphaTreshold) discard;
	FinalColor = vec4(1.0, 1.0, 1.0, 1.0);
}