#start VERTEX

void main() { }

#start GEOMETRY

#include "point2triangle.h"

#start FRAGMENT

#include "common.h"

in vec2 POS;
in vec2 UV;

layout(binding = 0) uniform sampler2D u_Texture;
layout(binding = 1) uniform sampler2D u_Normal;

uniform int u_NumLightSources;
uniform vec2 u_LightSources[MAX_LIGHT_SOURCES];

uniform bool u_NormalEnabled;

uniform float u_UVScale = 1.0;
uniform vec2 u_UVOffset = vec2(0.0,0.0);

uniform mat3 u_View;

layout(location = 0) out vec4 FinalColor;

const float alphaTreshold = 0.01;

void main()
{
	vec2 uv = UV + u_UVOffset;
	uv *= u_UVScale;

	vec4 tex = texture(u_Texture, uv);
	if (tex.a < alphaTreshold) discard;

	if (u_NormalEnabled)
	{
		float normalMask = 0.0;
		float lengthSum = 0.0;
		float lengths[MAX_LIGHT_SOURCES];
		vec2 dirs[MAX_LIGHT_SOURCES];
		vec2 normal = 2.0 * texture(u_Normal, uv).rg - 1.0;

		for(int i=0;i<u_NumLightSources;i++)
		{
			vec2 lightSource = (vec3(u_LightSources[i],1.0) * u_View).xy;
			vec2 dir = lightSource - POS;
			lengths[i] = 1.0/length(dir);
			dirs[i] = normalize(dir);
			lengthSum += lengths[i];
		}

		for(int i=0;i<u_NumLightSources;i++)
		{
			float lightMask = dot(dirs[i], normal) - 0.5;
			float lightFactor = (lengths[i]/lengthSum)*0.8;
			normalMask += lightFactor * lightMask;
		}

		tex.rgb += normalMask;
		tex.rgb = clamp(tex.rgb,0.0,1.0);

	}

	FinalColor = tex;
}