#include "LightOcclusionRenderer.h"

#include <algorithm>

#include "util/Profiler.h"

#include "gfx/GLCore.h"
#include "gfx/SceneCuller.h"
#include "scene/Scene.h"
#include "scene/Entity.h"

#include "gfx/DebugRenderer.h"

#define ENABLE_SAMPLE_MAP
//#define SORT_RAYS

extern bool CreateShader(const std::string& name, Shader*& shader);

static Vec2 angleComparatorRef = VEC2_ZERO;
bool angleComparator(const Vec4& l, const Vec4& r)
{
    Vec2 a = Vec2(l.x, l.y) - angleComparatorRef;
    Vec2 b = Vec2(r.x, r.y) - angleComparatorRef;
    return glm::atan(a.y, a.x) < glm::atan(b.y, b.x);
}

LightOcclusionRenderer::LightOcclusionRenderer()
{
    m_OcclusionLines = new UniformBuffer(sizeof(Vec4), MAX_LINE_SEGMENTS);
    m_RayQueryBuffer = new UniformBuffer(sizeof(Vec4), MAX_RAY_QUERIES);

    m_IntersectionBuffer = new ShaderStorageBuffer(sizeof(Vec4), MAX_RAY_QUERIES+1);

    std::vector<unsigned int> occlusionIndices;
    occlusionIndices.resize(3 * MAX_RAY_QUERIES);
    unsigned int currVert = 0;
    for (size_t i = 0; i < 3 * MAX_RAY_QUERIES; i+=3,currVert++)
    {
        occlusionIndices[i] = currVert;
        occlusionIndices[i + 1] = currVert + 1;
        occlusionIndices[i + 2] = MAX_RAY_QUERIES;
    }

    m_OcclusionMesh = new ShaderInput(m_IntersectionBuffer->GetBuffer(), occlusionIndices);

    m_OcclusionMaskFB = new Framebuffer(SCREEN_WIDTH, SCREEN_HEIGHT);
    m_OcclusionMaskFinal = new Framebuffer(SCREEN_WIDTH, SCREEN_HEIGHT);

    m_OcclusionMeshOutput = new ShaderStorageBuffer(sizeof(Vec4), MAX_OCCLUSION_MESH_SIZE);
}

LightOcclusionRenderer::~LightOcclusionRenderer()
{
    delete m_BlurShader;
    delete m_OcclusionMaskFinal;
    delete m_OcclusionMaskFB;
    delete m_OcclusionMeshGenShader;
    delete m_OcclusionMeshOutput;
    delete m_ShadowmapShader;
    delete m_OcclusionMesh;
    delete m_RayQueryBuffer;
    delete m_OcclusionShader;
    delete m_OcclusionLines;
    delete m_IntersectionBuffer;
}

void LightOcclusionRenderer::CompileShaders()
{
    static std::string shader_path = "occlusion/";

    CreateShader(shader_path + "light_occlusion", m_OcclusionShader);
    CreateShader(shader_path + "occlusion_mesh_gen", m_OcclusionMeshGenShader);
    CreateShader(shader_path + "shadowmap", m_ShadowmapShader);
    CreateShader("blur", m_BlurShader);
}

void LightOcclusionRenderer::OnOccluderAdded(Entity* e)
{
    ASSERT(e->GetDrawFlags().occluder);

    OcclusionProperties& props = e->GetOcclusionProperties();

    switch (props.shape)
    {
    case OccluderShape::Mesh:
    {
        int meshSize = 20 * (int) pow(2, props.meshLod);
        if (meshSize > 2*MAX_OCCLUSION_MESH_SIZE)
        {
            LOG("[Warrning][LightOcclusionRenderer] Occlusion mesh size is too big!");
            meshSize = 2*MAX_OCCLUSION_MESH_SIZE;
        }

        {
            m_OcclusionMeshGenShader->Bind();
            e->GetTexture()->Bind(0);
            m_OcclusionMeshOutput->Bind(1);
            m_OcclusionMeshGenShader->SetUniform("u_MeshSize", meshSize);
            GLFunctions::Dispatch(meshSize / 2);
        }

        GLFunctions::MemoryBarrier(BarrierType::ShaderStorage);
        OcclusionMesh& mesh = m_OcclusionMeshPool[e];
        PopulateOcclusionMesh(mesh, meshSize);

    } break;
    case OccluderShape::Rect:
    {
        static OcclusionMesh rectMesh = { Vec2(-1.0,-1.0),Vec2(-1.0,1.0),Vec2(1.0,1.0),Vec2(1.0,-1.0) };
        m_OcclusionMeshPool[e] = rectMesh;
    } break;
    default:
        NOT_IMPLEMENTED;
        break;
    }
}

void LightOcclusionRenderer::OnOccluderRemoved(Entity* e)
{
    ASSERT(e->GetDrawFlags().occluder);
    m_OcclusionMeshPool[e].clear();
}

void LightOcclusionRenderer::RenderOcclusion(CulledScene& scene)
{
    PROFILE_SCOPE("Render occlusion");

    size_t numEmitters = scene.GetEmitters().size();
    if (numEmitters < 1) return;

    const Camera& cam = scene.GetCamera();

    SetupBuffers(scene);

#ifdef ENABLE_SAMPLE_MAP
    std::map<EntityInstance*, unsigned> lightSampleMap = DivideSamples(scene);
#endif // ENABLE_SAMPLE_MAP

    m_OcclusionMaskFB->Clear();

    for (CulledEntity* ce : scene.GetEmitters())
    {
        Entity* emitter = ce->GetEntity();
        for (EntityInstance* emitter_ei : ce->GetInstances())
        {
#ifdef ENABLE_SAMPLE_MAP
            unsigned numLightSamples = lightSampleMap[emitter_ei];
#else
    #ifdef DEBUG
            unsigned numLightSamples = m_DebugOptions & OcclusionDebug_SimpleLightMask ? 1 : 6;
    #else
            unsigned numLightSamples = 6;
    #endif // DEBUG
#endif // ENABLE_SAMPLE_MAP

            Vec2 emitterPos = emitter_ei->GetPosition();
            emitterPos = cam.GetViewSpacePosition(emitterPos);
            m_CurrentQuery.color = emitter->GetEmissionProperties().color;
            m_CurrentQuery.radius = emitter->GetEmissionProperties().radius;
            m_CurrentQuery.strength = (1.0f / numLightSamples * 1.3f);

#ifdef SORT_RAYS
            Vec4* rayQueryBuffer = (Vec4*)m_RayQueryBuffer->Map(true);
            angleComparatorRef = emitterPos;
            std::sort(rayQueryBuffer, rayQueryBuffer + m_RayCount, angleComparator);
            m_RayQueryBuffer->Unmap();
#endif // SORT_RAYS

            for (unsigned i = 0; i < numLightSamples; i++)
            {
                const float angle = i * 2.0f * 3.1415f / numLightSamples;
                if (numLightSamples == 1)
                    m_CurrentQuery.position = emitterPos;
                else
                    m_CurrentQuery.position = emitterPos + Vec2(cos(angle), sin(angle)) * m_CurrentQuery.radius;

                LightOcclusion(scene);
                SortIntersections();
                RenderOcclusionMask(scene);
            }
        }
    }

#ifdef DEBUG
    if(!(m_DebugOptions & OcclusionDebug_SimpleLightMask))
#endif // DEBUG
    BlurMask();

    DrawDebug(scene);
}

void LightOcclusionRenderer::SetupBuffers(CulledScene& scene)
{
    PROFILE_SCOPE("Setup buffers");

    const Vec2 epsilon = Vec2(0.01f);
    const Mat3 view = scene.GetCamera().GetTransformation();

    std::vector<Vec4> lines;
    lines.reserve(m_OcclusionLineCount);
    m_OcclusionLineCount = 0;

#ifdef DEBUG
    if (!(m_DebugOptions & OcclusionDebug_DisableAngledRays))
#endif // DEBUG
    {
        // Angled rays
        for (size_t i = 0; i < NUM_ANGLED_RAYS; i++)
        {
            float angle = i * (2.0f * 6.283f / NUM_ANGLED_RAYS);
            Vec4 dir = Vec4(cos(angle), sin(angle), 0.0f, 0.0f);
            m_RayQuery.push_back(dir);
        }
    }

    for (CulledEntity* ce : scene.GetOccluders())
    {
        Entity* e = ce->GetEntity();
        const OcclusionMesh& mesh = m_OcclusionMeshPool[e];

        for (EntityInstance* ei : ce->GetInstances())
        {
            const Mat3 transform = ei->GetTransformation() * view;
            for (size_t i = 0; i < mesh.size(); i++)
            {
                m_OcclusionLineCount++;

                const size_t next_i = i + 1 == mesh.size() ? 0 : i + 1;
                const Vec2 p = mesh[i];
                const Vec2 next_p = mesh[next_i];

                // Line
                const Vec3 a = Vec3(p, 1.0) * transform;
                const Vec3 b = Vec3(next_p, 1.0) * transform;
                const Vec4 lineSegment = Vec4(a.x, a.y, b.x, b.y);
                lines.push_back(lineSegment);

                // Ray
                const Vec2 r1 = Vec2(a.x, a.y) - epsilon;
                const Vec2 r2 = Vec2(a.x, a.y) + epsilon;
                m_RayQuery.push_back({r1.x, r1.y, 0.0, 0.0});
                m_RayQuery.push_back({r2.x, r2.y, 0.0, 0.0});
            }
        }
    }

    // Upload rays
    m_RayCount = m_RayQuery.size();
    m_RayQueryBuffer->UploadData(m_RayQuery.data(), 0, m_RayCount);
    m_RayQuery.clear();

    // Upload lines
    m_OcclusionLines->UploadData(lines.data(), 0, m_OcclusionLineCount);
}

std::map<EntityInstance*, unsigned> LightOcclusionRenderer::DivideSamples(CulledScene& scene)
{
    PROFILE_SCOPE("Divide samples");

    std::map<EntityInstance*, unsigned> result;
    std::vector<EntityInstance*> emitters;

    const Vec2 camPos = -scene.GetCamera().position;

    for (CulledEntity* e : scene.GetEmitters())
    {
        for (EntityInstance* ei : e->GetInstances())
        {
            emitters.push_back(ei);
        }
    }
    
    bool calculateAgain = false;
    do
    {
        calculateAgain = false;
        std::map<EntityInstance*, float> distancesInv;
        float sumDistanceInv = 0.0f;
        for (EntityInstance* ei : emitters)
        {
            float d_inv = 1.0f / glm::length(camPos - ei->GetPosition());
            distancesInv[ei] = d_inv;
            sumDistanceInv += d_inv;
        }

        for (auto it = distancesInv.begin(); it != distancesInv.end(); ++it)
        {
            unsigned num = (unsigned)floor(it->second / sumDistanceInv * (float)MAX_LIGHT_SAMPLES);
            result[it->first] = MIN(num, OPTIMAL_LIGHT_SAMPLES);
            if (num > OPTIMAL_LIGHT_SAMPLES)
            {
                auto f = std::find(emitters.begin(), emitters.end(), it->first);
                ASSERT(f != emitters.end());
                emitters.erase(f);
                calculateAgain = true;
                break;
            }
        }

    } while (calculateAgain);

    return result;
}

void LightOcclusionRenderer::RenderOcclusionMask(CulledScene& scene)
{
    PROFILE_SCOPE("Occlusion mask");

    Vec3 attenuation = scene.GetLightAttenuation();

#ifdef DEBUG
    if (m_DebugOptions & OcclusionDebug_SimpleLightMask)
    {
        m_OcclusionMaskFinal->ClearAndBind();
        attenuation = { 1.0,0.0,0.0 };
    }
    else
#endif // DEBUG
        m_OcclusionMaskFB->Bind();

    m_ShadowmapShader->Bind();
    m_OcclusionMesh->Bind();
    GLFunctions::AlphaBlending(true);
    m_ShadowmapShader->SetUniform("u_MaskStrength", m_CurrentQuery.strength);
    m_ShadowmapShader->SetUniform("u_LightPos", m_CurrentQuery.position);
    m_ShadowmapShader->SetUniform("u_LightColor", m_CurrentQuery.color);
    m_ShadowmapShader->SetUniform("u_LightRadius", m_CurrentQuery.radius);
    m_ShadowmapShader->SetUniform("u_Attenuation", attenuation);
    m_OcclusionMesh->Bind(); // HACK: For some reason at first draw call something is bound inbetween
    GLFunctions::DrawIndexed(m_RayCount * 3);
    GLFunctions::AlphaBlending(false);
    m_OcclusionMaskFB->Unbind();
}

bool intersect(Vec2& intersection, Vec2 a1, Vec2 a2, Vec2 b1, Vec2 b2)
{
    float x = a1.x, y = a1.y, dx = a2.x, dy = a2.y;
    float x1 = b1.x, y1 = b1.y, x2 = b2.x, y2 = b2.y;
    float r, s, d;

    if (dy / dx != (y2 - y1) / (x2 - x1))
    {
        d = ((dx * (y2 - y1)) - dy * (x2 - x1));
        if (d != 0)
        {
            r = (((y - y1) * (x2 - x1)) - (x - x1) * (y2 - y1)) / d;
            s = (((y - y1) * dx) - (x - x1) * dy) / d;
            if (r >= 0 && r <= 1 && s >= 0 && s <= 1)
            {
                intersection.x = x + r * dx;
                intersection.y = y + r * dy;
                return true;
            }
        }
    }
    return false;
}

bool findIntersecton(Vec2& intersection, std::vector<Vec2> a, std::vector<Vec2> b)
{
    // TODO: Optimize this

    for (size_t i = 1; i < a.size(); i++)
    {
        for (size_t j = 1; j < b.size(); j++)
        {
            if (intersect(intersection, a[i-1], a[i], b[j-1], b[j])) return true;
        }
    }
    return false;
}

void LightOcclusionRenderer::PopulateOcclusionMesh(OcclusionMesh& mesh, int meshSize)
{
    mesh.clear();
    std::vector<Vec2> vertices[4]; // Left, top, right, down

    int oBufferSize = meshSize;
    Vec4* ptr = (Vec4*)m_OcclusionMeshOutput->Map();
    for (int i = 0; i < oBufferSize / 2; i++) // L
    {
        Vec4 value = ptr[i];
        Vec2 a = Vec2(value.x, value.y);
        if (a.x >= -1.0 && a.x <= 1.0) vertices[0].push_back(a);
    }
    for (int i = oBufferSize / 2; i < oBufferSize; i++) // T
    {
        Vec4 value = ptr[i];
        Vec2 a = Vec2(value.z, value.w);
        if (a.y >= -1.0 && a.y <= 1.0) vertices[1].push_back(a);
    }
    for (int i = oBufferSize / 2 - 1; i > 0; i--) // R
    {
        Vec4 value = ptr[i];
        Vec2 a = Vec2(value.z, value.w);
        if (a.x >= -1.0 && a.x <= 1.0) vertices[2].push_back(a);
    }
    for (int i = oBufferSize - 1; i > oBufferSize / 2; i--) // D
    {
        Vec4 value = ptr[i];
        Vec2 a = Vec2(value.x, value.y);
        if (a.y >= -1.0 && a.y <= 1.0) vertices[3].push_back(a);
    }
    m_OcclusionMeshOutput->Unmap();

    // Intersectons
    Vec2 intersection[4];
    bool has_intersection[4]; // intersection success
    has_intersection[0] = findIntersecton(intersection[0], vertices[0], vertices[1]);
    has_intersection[1] = findIntersecton(intersection[1], vertices[1], vertices[2]);
    has_intersection[2] = findIntersecton(intersection[2], vertices[2], vertices[3]);
    has_intersection[3] = findIntersecton(intersection[3], vertices[3], vertices[0]);

    for (Vec2 a : vertices[0]) // Left
    {
        if (has_intersection[3] && a.y < intersection[3].y) continue;
        if (has_intersection[0] && a.y > intersection[0].y) break;
        mesh.push_back(a);
    }

    for (Vec2 a : vertices[1]) // Top
    {
        if (has_intersection[0] && a.x < intersection[0].x) continue;
        if (has_intersection[1] && a.x > intersection[1].x) break;
        mesh.push_back(a);
    }

    for (Vec2 a : vertices[2]) // Right
    {
        if (has_intersection[1] && a.y > intersection[1].y) continue;
        if (has_intersection[2] && a.y < intersection[2].y) break;
        mesh.push_back(a);
    }

    for (Vec2 a : vertices[3]) // Down
    {
        if (has_intersection[2] && a.x > intersection[2].x) continue;
        if (has_intersection[3] && a.x < intersection[3].x) break;
        mesh.push_back(a);
    }
}

void LightOcclusionRenderer::LightOcclusion(CulledScene& scene)
{
    PROFILE_SCOPE("Light occlusion");

    m_OcclusionShader->Bind();
    m_OcclusionShader->SetUniform("u_LightPosition", m_CurrentQuery.position);
    m_OcclusionShader->SetUniform("u_NumSegments", (int) m_OcclusionLineCount);
    m_IntersectionBuffer->Bind(1);
    m_OcclusionLines->Bind(2);
    m_RayQueryBuffer->Bind(3);
    GLFunctions::Dispatch(m_RayCount);
}

void LightOcclusionRenderer::SortIntersections()
{
    PROFILE_SCOPE("Sort intersections");

    Vec4* intersectionBuffer = (Vec4*)m_IntersectionBuffer->Map(true);
#ifndef SORT_RAYS
    angleComparatorRef = m_CurrentQuery.position;
    std::sort(intersectionBuffer, intersectionBuffer + m_RayCount, angleComparator);
#endif // SORT_RAYS
    intersectionBuffer[m_RayCount] = intersectionBuffer[0];
    intersectionBuffer[MAX_RAY_QUERIES] = { m_CurrentQuery.position, 0.0 ,0.0 };
    m_IntersectionBuffer->Unmap();
}

void LightOcclusionRenderer::BlurMask()
{
    PROFILE_SCOPE("Blur mask");

    GLFunctions::MemoryBarrier(BarrierType::Framebuffer);
    m_OcclusionMaskFinal->ClearAndBind();
    m_BlurShader->Bind();
    m_OcclusionMaskFB->BindTexture(0, 0);
    GLFunctions::DrawFC();
}

void LightOcclusionRenderer::DrawDebug(CulledScene& scene)
{
#ifdef DEBUG
    if (m_DebugOptions == 0) return;

    Camera& cam = scene.GetCamera();

    if (m_DebugOptions & OcclusionDebug_Rays)
    {
        GLFunctions::MemoryBarrier(BarrierType::UniformBuffer);
        Vec4* rayBuffer = (Vec4*) m_RayQueryBuffer->Map();
        for (size_t i=0; i<m_RayCount;i++)
        {
            Vec4 ray = rayBuffer[i];
            DebugRenderer::Get()->DrawLine(m_CurrentQuery.position - cam.position, Vec2(ray.x,ray.y) - cam.position, { 0.0,1.0,0.0 });
        }
        m_RayQueryBuffer->Unmap();
    }

    if (m_DebugOptions & OcclusionDebug_Mesh)
    {
        GLFunctions::MemoryBarrier(BarrierType::UniformBuffer);
        Vec4* linesBuffer = (Vec4*) m_OcclusionLines->Map();
        for (size_t i = 0; i < m_OcclusionLineCount; i++)
        {
            Vec4 ls = linesBuffer[i] - Vec4(cam.position,cam.position);
            DebugRenderer::Get()->DrawLine({ ls.x,ls.y }, { ls.z, ls.w}, {0.0,0.0,1.0});
            DebugRenderer::Get()->DrawPoint({ ls.x,ls.y }, { 0.0,0.0,1.0 });
        }
        m_OcclusionLines->Unmap();
    }

    if (m_DebugOptions & OcclusionDebug_Intersections)
    {
        GLFunctions::MemoryBarrier(BarrierType::ShaderStorage);
        Vec4* intersectionBuffer = (Vec4*)m_IntersectionBuffer->Map();
        for (size_t i = 0; i < m_RayCount; i++)
        {
            Vec4 intersection = intersectionBuffer[i];
            DebugRenderer::Get()->DrawPoint(Vec2(intersection.x, intersection.y) - cam.position, { 1.0,0.0,0.0 });
        }
        m_IntersectionBuffer->Unmap();
    }
#endif // DEBUG
}