#start VERTEX

layout(location = 0) in vec2 in_Position;
layout(location = 1) in vec2 in_UV;

uniform mat3 u_Transform;

out vec2 POS;
out vec2 UV;

void main()
{
	vec3 pos = vec3(in_Position, 1.0) * u_Transform;
	POS = pos.xy;
	gl_Position = vec4(pos.xy, 0.0, 1.0);
	UV = in_UV;
}

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