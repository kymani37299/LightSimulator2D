#version 430

#define NUM_INTERSECTIONS 30
#define NUM_TRIANGLED_INTERSECTIONS ((NUM_INTERSECTIONS-1)*3)

layout(local_size_x = 1) in;

uniform vec2 lightPosition;

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
    triangledIntersections[3 * id] = intersections[id];
    triangledIntersections[3 * id + 1] = lightPosition;
    triangledIntersections[3 * id + 2] = intersections[id + 1];
}