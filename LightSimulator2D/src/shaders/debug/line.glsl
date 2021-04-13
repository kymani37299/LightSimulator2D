#start VERTEX

layout(location = 0) in vec2 in_Position;
layout(location = 1) in vec2 in_UV;

uniform mat3 u_View;
uniform vec2 u_Begin;
uniform vec2 u_End;

out vec2 UV_NORM;
out vec2 LINE_BEGIN;
out vec2 LINE_END;

void main()
{
	LINE_BEGIN = (vec3(u_Begin,1.0) * u_View).xy;
	LINE_END = (vec3(u_End,1.0) * u_View).xy;
	gl_Position = vec4(in_Position, 0.0, 1.0);
	UV_NORM = 2.0*in_UV-1.0;
}

#start FRAGMENT

in vec2 UV_NORM;
in vec2 LINE_BEGIN;
in vec2 LINE_END;

uniform vec3 u_Color;

layout(location = 0) out vec4 FinalColor;

float line_mask(vec2 P, vec2 A, vec2 B, float r) 
{
    vec2 g = B - A;
    vec2 h = P - A;
    float d = length(h - g * clamp(dot(g, h) / dot(g,g), 0.0, 1.0));
	return smoothstep(r, 0.5*r, d);
}

const float line_thickness = 0.01;

void main()
{
	float line = line_mask(UV_NORM, LINE_BEGIN, LINE_END, line_thickness);
	FinalColor = vec4(u_Color,line);
}