#version 430

#include "common.h"

layout(local_size_x = 1) in;

layout(binding = 0) uniform sampler2D u_Texture;

uniform int u_MeshSize;

layout(std140, binding = 1) buffer writeonly OcclusionMeshOutput
{
    vec4 w_OcclusionMesh[MAX_OCCLUSION_MESH_SIZE];
};

const float alphaTreshold = 0.01;

void main()
{
    uint id = gl_GlobalInvocationID.x;
    float step = 0.01f;
    float cStep = 4.0f / u_MeshSize;

    if (id < u_MeshSize / 4)
    {
        vec2 uv = vec2(0.0, id * cStep);

        while (uv.x < 1.0 && texture(u_Texture, uv).a < alphaTreshold) uv.x += step;
        w_OcclusionMesh[id].xy = 2.0 * uv - 1.0;

        uv.x = 1.0;
        while (uv.x > 0.0 && texture(u_Texture, uv).a < alphaTreshold) uv.x -= step;
        w_OcclusionMesh[id].zw = 2.0 * uv - 1.0;
    }
    else
    {
        vec2 uv = vec2((id- u_MeshSize / 4) * cStep, 0.0);

        while (uv.y < 1.0 && texture(u_Texture, uv).a < alphaTreshold) uv.y += step;
        w_OcclusionMesh[id].xy = 2.0 * uv - 1.0;

        uv.y = 1.0;
        while (uv.y > 0.0 && texture(u_Texture, uv).a < alphaTreshold) uv.y -= step;
        w_OcclusionMesh[id].zw = 2.0 * uv - 1.0;
    }

}