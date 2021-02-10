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

	// RAY in parametric: Point + Direction*T1
	float r_px = ray.x;
	float r_py = ray.y;
	float r_dx = ray.z - ray.x;
	float r_dy = ray.w - ray.y;

	// SEGMENT in parametric: Point + Direction*T2
	float s_px = segment.x;
	float s_py = segment.y;
	float s_dx = segment.z - segment.x;
	float s_dy = segment.w - segment.y;

	// Are they parallel? If so, no intersect
	float r_mag = sqrt(r_dx * r_dx + r_dy * r_dy);
	float s_mag = sqrt(s_dx * s_dx + s_dy * s_dy);

	if (r_dx / r_mag == s_dx / s_mag && r_dy / r_mag == s_dy / s_mag)
	{
		return;
	}

	// SOLVE FOR T1 & T2
	// r_px+r_dx*T1 = s_px+s_dx*T2 && r_py+r_dy*T1 = s_py+s_dy*T2
	// ==> T1 = (s_px+s_dx*T2-r_px)/r_dx = (s_py+s_dy*T2-r_py)/r_dy
	// ==> s_px*r_dy + s_dx*T2*r_dy - r_px*r_dy = s_py*r_dx + s_dy*T2*r_dx - r_py*r_dx
	// ==> T2 = (r_dx*(s_py-r_py) + r_dy*(r_px-s_px))/(s_dx*r_dy - s_dy*r_dx)
	float T2 = (r_dx * (s_py - r_py) + r_dy * (r_px - s_px)) / (s_dx * r_dy - s_dy * r_dx);
	float T1 = (s_px + s_dx * T2 - r_px) / r_dx;

	// Must be within parametic whatevers for RAY/SEGMENT
	if (T1 < 0) return;
	if (T2 < 0 || T2 > 1) return;

	if(T1 < intersection.z)
    {
		intersection = vec3(r_px + r_dx * T1, r_py + r_dy * T1, T1);
    }
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
	intersections[gl_GlobalInvocationID.x] = closestIntersect.xy;
}