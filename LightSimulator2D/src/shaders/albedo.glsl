#start VERTEX

void main() { }

#start GEOMETRY

#include "point2triangle.h"

#start FRAGMENT

in vec2 POS;
in vec2 UV;

layout(binding = 0) uniform sampler2D u_Texture;
layout(binding = 1) uniform sampler2D u_Normal;

uniform bool u_NormalEnabled;
uniform vec2 u_LightSource;

layout(location = 0) out vec4 FinalColor;

const float alphaTreshold = 0.01;

void main()
{
	vec4 tex = texture(u_Texture, UV);
	if (tex.a < alphaTreshold) discard;

	if (u_NormalEnabled)
	{
		vec2 dir = normalize(u_LightSource - POS);
		vec2 normal = 2.0 * texture(u_Normal, UV).rg - 1.0;
		float lightMask = dot(dir, normal) - 0.5;
		
		tex.rgb += lightMask;

	}
	
	tex.rgb = clamp(tex.rgb,0.0,1.0);
	FinalColor = tex;
}