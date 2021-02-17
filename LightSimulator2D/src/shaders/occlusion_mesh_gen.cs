#version 430

#include "common.h"

layout(local_size_x = 1) in;

uniform sampler2D u_Texture;

layout(std140, binding = 1) buffer writeonly OcclusionMeshOutput
{
    vec4 occlusionMesh[OCCUSION_MESH_SIZE/2];
};

const float alphaTreshold = 0.01;

void main()
{
    uint id = gl_GlobalInvocationID.x;
    float step = 0.01f;
    vec2 uv = vec2(0.0, id * 2.0f / OCCUSION_MESH_SIZE);

    while (uv.x < 1.0 && texture(u_Texture, uv).a < alphaTreshold) uv.x += step;
    occlusionMesh[id].xy = 2.0*uv-1.0;

    uv.x = 1.0;
    while (uv.x > 0.0 && texture(u_Texture, uv).a < alphaTreshold) uv.x -= step;
    occlusionMesh[id].zw = 2.0 * uv - 1.0;
}