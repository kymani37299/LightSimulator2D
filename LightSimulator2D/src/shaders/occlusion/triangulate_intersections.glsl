#start COMPUTE

#include "common.h"

layout(local_size_x = 1) in;

uniform vec2 u_LightPosition;
uniform int u_NumIntersections;

layout(std140, binding = 1) buffer readonly IntersectionsBuffer
{
    vec4 r_Intersections[MAX_RAY_QUERIES];
};

layout(std140, binding = 2) buffer writeonly TriangledIntersectionsBuffer
{
    vec4 w_TriangledIntersections[MAX_RAY_QUERIES*3];
};

void main()
{
	int id = int(gl_GlobalInvocationID.x);
    int next_id = id + 1 == u_NumIntersections ? 0 : id + 1;
    w_TriangledIntersections[3 * id].xy = r_Intersections[id].xy;
    w_TriangledIntersections[3 * id + 1].xy = u_LightPosition;
    w_TriangledIntersections[3 * id + 2].xy = r_Intersections[next_id].xy;
}