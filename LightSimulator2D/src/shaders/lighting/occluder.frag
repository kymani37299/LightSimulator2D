#version 430

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

	float lightMask = 0.0;

	if (u_NormalEnabled)
	{
		vec2 dir = normalize(u_LightSource - POS);
		vec2 normal = 2.0 * texture(u_Normal, UV).rg - 1.0;
		lightMask = dot(dir, normal);
		lightMask -= 0.5;
	}

	tex.rgb *= tex.a;
	tex.rgb += lightMask;
	tex.rgb = clamp(tex.rgb, 0.0, 1.0);
	tex.a = 1.0;
	FinalColor = tex;
}