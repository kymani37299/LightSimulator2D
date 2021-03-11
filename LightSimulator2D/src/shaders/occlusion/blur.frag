#version 430

#define PI2 6.28318530718

in vec2 UV;

layout(binding = 0) uniform sampler2D u_Input;

layout(location = 0) out vec4 FinalColor;

const float directions = 16.0; // BLUR DIRECTIONS (Default 16.0 - More is better but slower)
const float quality = 3.0; // BLUR QUALITY (Default 3.0 - More is better but slower)
const float size = 8.0; // BLUR SIZE (Radius)

void main()
{
    vec2 screen_size = vec2(1600, 900); // TODO: Get this from common params
    vec2 radius = size / screen_size;
    vec4 color = texture(u_Input, UV);

    for (float d = 0.0; d < PI2; d += PI2 / directions)
    {
        for (float i = 1.0 / quality; i <= 1.0; i += 1.0 / quality)
        {
            color += texture(u_Input, UV + vec2(cos(d), sin(d)) * radius * i);
        }
    }

    color /= quality * directions - 15.0;
    FinalColor = color;
}