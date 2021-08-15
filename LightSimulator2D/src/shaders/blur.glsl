#start VERTEX

layout(location = 0) in vec2 in_Position;
layout(location = 1) in vec2 in_UV;

out vec2 UV;

void main()
{
	gl_Position = vec4(in_Position, 0.0, 1.0);
	UV = in_UV;
}

#start FRAGMENT

#include "common.h"

#define PI2 6.28318530718

in vec2 UV;

layout(binding = 0) uniform sampler2D u_Input;

layout(location = 0) out vec4 FinalColor;

// Source: https://www.shadertoy.com/view/Xltfzj

const float directions = 8.0; // BLUR DIRECTIONS (Default 16.0 - More is better but slower)
const float quality = 2.0; // BLUR QUALITY (Default 3.0 - More is better but slower)
const float size = 32.0; // BLUR SIZE (Radius)
const float distance_falloff = 15.0;

void main()
{
    vec4 color = texture(u_Input, UV);
    float blur_size = size - distance_falloff * length(color);
    vec2 radius = blur_size / vec2(SCREEN_WIDTH, SCREEN_HEIGHT);

    for (float d = 0.0; d < PI2; d += PI2 / directions)
    {
        for (float i = 1.0 / quality; i <= 1.0; i += 1.0 / quality)
        {
            color += texture(u_Input, UV + vec2(cos(d), sin(d)) * radius * i);
        }
    }

    color /= quality * directions - directions + 1.0;
    FinalColor = color;
}