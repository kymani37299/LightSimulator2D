#version 430

#include "common.h"

layout(local_size_x = 1) in;

uniform vec2 lightPosition;
uniform int numIntersections;

layout(std140, binding = 1) buffer readonly IntersectionsBuffer
{
    vec2 intersections[NUM_INTERSECTIONS];
};

layout(std140, binding = 2) buffer writeonly TriangledIntersectionsBuffer
{
    vec2 triangledIntersections[NUM_TRIANGLED_INTERSECTIONS];
};

void main()
{
	int id = int(gl_GlobalInvocationID.x);
    int next_id = id + 1 == numIntersections ? 0 : id + 1;
    triangledIntersections[3 * id] = intersections[id];
    triangledIntersections[3 * id + 1] = lightPosition;
    triangledIntersections[3 * id + 2] = intersections[next_id];
}