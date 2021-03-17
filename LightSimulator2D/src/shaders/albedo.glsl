#start VERTEX

void main() { }

#start GEOMETRY

#include "point2quad.h"

#start FRAGMENT

#include "common.h"
#include "attenuation.h"

in vec2 POS;
in vec2 UV;

layout(binding = 0) uniform sampler2D u_Texture;
layout(binding = 1) uniform sampler2D u_Normal;

uniform int u_NumLightSources;
uniform vec2 u_LightSources[MAX_LIGHT_SOURCES];

uniform bool u_NormalEnabled;
uniform bool u_DistanceBasedLight;

uniform vec2 u_UVScale = vec2(1.0,1.0);
uniform vec2 u_UVOffset = vec2(0.0,0.0);

uniform mat3 u_View;

layout(location = 0) out vec4 FinalColor;

const float alphaTreshold = 0.01;

const vec3 ambientLight = vec3(0.0,0.05,0.0);

void main()
{
	vec2 uv = UV + u_UVOffset;
	uv *= u_UVScale;

	vec4 tex = texture(u_Texture, uv);
	if (tex.a < alphaTreshold) discard;

	if(u_DistanceBasedLight)
	{
		float lightFactor = 0.0;
		for(int i=0;i<u_NumLightSources;i++)
		{
			vec2 lightSource = (vec3(u_LightSources[i],1.0) * u_View).xy;
			float dist = length(POS - lightSource);
			lightFactor += Attenuate(dist);
		}
		lightFactor = clamp(lightFactor,0.0,1.0);
		tex.rgb = ambientLight + tex.rgb*lightFactor;
	}

	if (u_NormalEnabled)
	{
		vec2 normal = 2.0 * texture(u_Normal, uv).rg - 1.0f;

		vec3 normalFactor = vec3(0.0,0.0,0.0);
		for(int i=0;i<u_NumLightSources;i++)
		{
			vec2 lightSource = (vec3(u_LightSources[i],1.0) * u_View).xy;
			vec2 dir = normalize(lightSource - POS);
			float dist = length(POS - lightSource);
			float attenuation = Attenuate(dist);
			tex.rgb += attenuation*dot(dir,normal);
		}
	}

	tex.rgb = clamp(tex.rgb,0.0,1.0);
	FinalColor = tex;
}