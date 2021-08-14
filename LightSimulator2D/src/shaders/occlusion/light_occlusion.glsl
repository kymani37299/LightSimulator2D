#start COMPUTE

#include "common.h"

#define PI2 6.2830

layout(local_size_x = 1) in;

uniform vec2 u_LightPosition;
uniform int u_NumSegments;

layout(std140, binding = 1) writeonly buffer IntersectionsBuffer
{
    vec4 w_Intersections[MAX_RAY_QUERIES];
};

layout(std140, binding = 2) uniform LineSegmentsBuffer
{
    vec4 r_LineSegments[MAX_LINE_SEGMENTS];
};

layout(std140, binding = 3) uniform RayQueryBuffer
{
    vec4 r_Rays[MAX_RAY_QUERIES];
};

void calcIntersection(out vec3 intersection, vec4 ray, vec4 segment)
{
    float x = ray.x, y = ray.y, dx = ray.z, dy = ray.w;
    float x1 = segment.x, y1 = segment.y, x2 = segment.z, y2 = segment.w;
    float r, s, d;

    if (dy / dx != (y2 - y1) / (x2 - x1))
    {
        d = ((dx * (y2 - y1)) - dy * (x2 - x1));
        if (d != 0.0)
        {
            r = (((y - y1) * (x2 - x1)) - (x - x1) * (y2 - y1)) / d;
            s = (((y - y1) * dx) - (x - x1) * dy) / d;
            if (r >= 0.0 && s >= 0.0 && s <= 1.0 && r < intersection.z)
            {
                intersection.x = x + r * dx;
                intersection.y = y + r * dy;
                intersection.z = r;
            }
        }
    }
}

const float areaSize = EMITTER_AREA;

void intersectScreen(out vec3 intersection, vec4 ray)
{
    calcIntersection(intersection, ray, areaSize * vec4(-1.0, -1.0, 1.0, -1.0));
    calcIntersection(intersection, ray, areaSize * vec4(1.0, -1.0, 1.0, 1.0));
    calcIntersection(intersection, ray, areaSize * vec4(1.0, 1.0, -1.0, 1.0));
    calcIntersection(intersection, ray, areaSize * vec4(-1.0, 1.0, -1.0, -1.0));
}

void main()
{
    vec2 rayDir = normalize(r_Rays[gl_GlobalInvocationID.x].xy - u_LightPosition);
    vec4 ray = vec4(u_LightPosition, rayDir);
    vec3 closestIntersect = vec3(1000.0f);

    for (int i = 0; i < u_NumSegments; i++)
    {
        calcIntersection(closestIntersect, ray, r_LineSegments[i]);
    }

    if (closestIntersect.z == 1000.0f)
    {
        intersectScreen(closestIntersect, ray);
    }

    w_Intersections[gl_GlobalInvocationID.x].xy = closestIntersect.xy;
}