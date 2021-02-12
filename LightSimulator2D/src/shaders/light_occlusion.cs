#version 430

#define PI2 6.2830
#define NUM_INTERSECTIONS 30
#define MAX_LINE_SEGMENTS 200

layout(local_size_x = 1) in;

uniform vec2 lightPosition;
uniform int numSegments;

layout(std140, binding = 1) buffer IntersectionsBuffer
{
    vec2 intersections[NUM_INTERSECTIONS];
};

layout(std140, binding = 2) uniform LineSegmentsBuffer
{
    vec4 lineSegments[MAX_LINE_SEGMENTS];
};

void calcIntersection(out vec3 intersection, vec4 ray, vec4 segment)
{
	// Parametric form : Origin + t*Direction
	// ro + t1*rd
	vec2 ro = ray.xy;
	vec2 rd = ray.zw;

	// a + t2*b
	vec2 a = segment.xy;
	vec2 b = segment.zw;

	float t2_1 = rd.x * (a.y - ro.y) + rd.y * (ro.x - a.x);
	float t2_2 = b.x * rd.y - b.y * rd.x;
	float t2 = t2_1 / t2_2;

	float t1_1 = a.x + b.x * t2 - ro.x;
	float t1_2 = rd.x;
	float t1 = t1_1 / t1_2;

	// Parallel lines
	if (t1_2 == 0 || t2_2 == 0) return;

	// Intersect test
	if(t1 > 0.0 && t2 > 0.0 && t2 < 1.0)
    {
		vec2 p = ro + t1 * rd;
		intersection.xy = p;
		intersection.z = length(p - ro);
    }
}

// TODO: Optimize this
void intersectScreen(out vec3 intersection, vec4 ray)
{
	calcIntersection(intersection, ray, vec4(-1.0, -1.0, 1.0, -1.0));
	calcIntersection(intersection, ray, vec4(1.0, -1.0, 1.0, 1.0));
	calcIntersection(intersection, ray, vec4(1.0, 1.0, -1.0, 1.0));
	calcIntersection(intersection, ray, vec4(-1.0, 1.0, -1.0, -1.0));
}

void main()
{
	float angle = gl_GlobalInvocationID.x * (PI2 / NUM_INTERSECTIONS);
	float dx = cos(angle);
	float dy = sin(angle);

	vec3 closestIntersect = vec3(1000.0);
	vec4 ray = vec4(lightPosition, lightPosition + vec2(dx, dy));

	for (int i = 0; i < numSegments; i++)
	{
		calcIntersection(closestIntersect, ray, lineSegments[i]);
	}

	if(closestIntersect.x == 1000.0)
    {
		intersectScreen(closestIntersect, ray);
	}

	intersections[gl_GlobalInvocationID.x] = closestIntersect.xy;
}