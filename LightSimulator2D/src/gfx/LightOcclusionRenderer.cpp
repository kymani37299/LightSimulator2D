#include "LightOcclusionRenderer.h"

#include "util/Profiler.h"

#include "core/Engine.h"
#include "input/Input.h"

#include "gfx/GLCore.h"
#include "scene/Entity.h"
#include "scene/Scene.h"

LightOcclusionRenderer::LightOcclusionRenderer()
{
#ifdef GPU_OCCLUSION
    m_OcclusionLines = new UniformBuffer(sizeof(Vec4), MAX_LINE_SEGMENTS);

    m_IntersectionBuffer = new ShaderStorageBuffer(sizeof(Vec4), NUM_INTERSECTIONS);

    m_TriangledIntersecitonsBuffer = new ShaderStorageBuffer(sizeof(Vec4), NUM_INTERSECTIONS * 3);

    m_RayQueryBuffer = new UniformBuffer(sizeof(Vec4), NUM_INTERSECTIONS);

    m_OcclusionMesh = m_TriangledIntersecitonsBuffer->AsShaderInput();
#endif
}

LightOcclusionRenderer::~LightOcclusionRenderer()
{
#ifdef GPU_OCCLUSION
    delete m_OcclusionMesh;
    delete m_RayQueryBuffer;
    delete m_TriangulationShader;
    delete m_OcclusionShader;
    delete m_OcclusionLines;
    delete m_IntersectionBuffer;
    delete m_TriangledIntersecitonsBuffer;
#endif
}

void LightOcclusionRenderer::RenderOcclusion(Scene* scene)
{
    LightOcclusion(scene);
    TriangulateMeshes();
}

unsigned LightOcclusionRenderer::SetupOcclusionMeshInput()
{
#ifdef GPU_OCCLUSION
    unsigned numIntersections = NUM_INTERSECTIONS * 3;
#else
    SAFE_DELETE(m_OcclusionMesh);
    unsigned numIntersections = m_TriangledIntersections.size();
    m_OcclusionMesh = new ShaderInput(m_TriangledIntersections);
#endif
    m_OcclusionMesh->Bind();

    return numIntersections;
}

void LightOcclusionRenderer::SetupLineSegments(Scene* scene)
{
    m_OcclusionLineCount = 0;
    for (size_t id = 0; id < scene->Size(); id++)
    {
        static std::vector<Vec2> aabbVertices = { {-1.0,-1.0},{1.0,-1.0},{1.0,1.0},{-1.0,1.0} };
        Mat3 transform = (*scene)[id].GetTransformation();

        for (size_t i = 0; i < aabbVertices.size(); i++)
        {
            Vec3 a = Vec3(aabbVertices[i], 1.0) * transform;
            size_t next_i = i + 1 == 4 ? 0 : i + 1;
            Vec3 b = Vec3(aabbVertices[next_i], 1.0) * transform;
            Vec4 lineSegment = Vec4(a.x, a.y, b.x, b.y);
#ifdef GPU_OCCLUSION
            m_OcclusionLines->UploadData(&lineSegment, m_OcclusionLineCount);
#else
            m_Segments[m_OcclusionLineCount] = lineSegment;
#endif
            m_OcclusionLineCount++;
        }
    }
}

#ifdef GPU_OCCLUSION
void LightOcclusionRenderer::SetupRayQuery()
{
    m_RayCount = 0;
    for (size_t i = 0; i < NUM_INTERSECTIONS; i++)
    {
        float angle = i * (2.0f * 6.283f / NUM_INTERSECTIONS);
        Vec4 dir = Vec4(cos(angle), sin(angle),0.0,0.0);
        m_RayQueryBuffer->UploadData(&dir, m_RayCount);
        m_RayCount++;
    }
}

void LightOcclusionRenderer::LightOcclusion(Scene* scene)
{
    PROFILE_SCOPE("Light occlusion");

    SetupLineSegments(scene);
    SetupRayQuery();
    m_LightSource = GameEngine::Get()->GetInput()->GetMousePosition();

    m_OcclusionShader->Bind();
    m_OcclusionShader->SetUniform("lightPosition", m_LightSource);
    m_OcclusionShader->SetUniform("numSegments", (int)m_OcclusionLineCount);
    m_IntersectionBuffer->Bind(1);
    m_OcclusionLines->Bind(2);
    m_RayQueryBuffer->Bind(3);
    GLFunctions::Dispatch(m_RayCount);
}

void LightOcclusionRenderer::TriangulateMeshes()
{
    PROFILE_SCOPE("Triangulate intersections");

    GLFunctions::MemoryBarrier(BarrierType::BufferUpdate); // Not sure if this is right barrier
    m_TriangulationShader->Bind();
    m_TriangulationShader->SetUniform("lightPosition", m_LightSource);
    m_TriangulationShader->SetUniform("numIntersections", (int) m_RayCount);
    m_IntersectionBuffer->Bind(1);
    m_TriangledIntersecitonsBuffer->Bind(2);
    GLFunctions::Dispatch(NUM_INTERSECTIONS*3);
}
#else

void calcIntersection(Vec3& intersection, Vec4 ray, Vec4 segment)
{
    float x = ray.x, y = ray.y, dx = ray.z, dy = ray.w;
    float x1 = segment.x, y1 = segment.y, x2 = segment.z, y2 = segment.w;
    float r, s, d;

    if (dy / dx != (y2 - y1) / (x2 - x1))
    {
        d = ((dx * (y2 - y1)) - dy * (x2 - x1));
        if (d != 0)
        {
            r = (((y - y1) * (x2 - x1)) - (x - x1) * (y2 - y1)) / d;
            s = (((y - y1) * dx) - (x - x1) * dy) / d;
            if (r >= 0 && s >= 0 && s <= 1 && r < intersection.z)
            {
                intersection.x = x + r * dx;
                intersection.y = y + r * dy;
                intersection.z = r;
            }
        }
    }
}

// TODO: Optimize this
void intersectScreen(Vec3& intersection, Vec4 ray)
{
    calcIntersection(intersection, ray, Vec4(-1.0, -1.0, 1.0, -1.0));
    calcIntersection(intersection, ray, Vec4(1.0, -1.0, 1.0, 1.0));
    calcIntersection(intersection, ray, Vec4(1.0, 1.0, -1.0, 1.0));
    calcIntersection(intersection, ray, Vec4(-1.0, 1.0, -1.0, -1.0));
}

void LightOcclusionRenderer::SetupRayQuery()
{
    ASSERT(m_RayQuery.size() == 0);

    for (size_t i = 0; i < NUM_INTERSECTIONS; i++)
    {
        float angle = i * (2.0f * 6.283f / NUM_INTERSECTIONS);
        m_RayQuery.push(Vec2(cos(angle), sin(angle)));
    }

    const Vec2 e = Vec2(0.01f);
    for (const Vec4& seg : m_Segments)
    {
        Vec2 p = Vec2(seg.x, seg.y);
        m_RayQuery.push(p - e);
        m_RayQuery.push(e);
        m_RayQuery.push(p + e);
    }
}

void LightOcclusionRenderer::LightOcclusion(Scene* scene)
{
    PROFILE_SCOPE("Light occlusion");

    m_Segments.clear();
    m_Segments.resize(4 * scene->Size());
    SetupLineSegments(scene);
    SetupRayQuery();
    m_LightSource = GameEngine::Get()->GetInput()->GetMousePosition();

    m_Intersections.clear();
    m_Intersections.reserve(m_RayQuery.size());

    while (!m_RayQuery.empty())
    {
        Vec4 ray = Vec4(m_LightSource, m_RayQuery.top());
        m_RayQuery.pop();

        Vec3 closestIntersect = Vec3(1000.0f);

        for (size_t i = 0; i < m_Segments.size(); i++)
        {
            calcIntersection(closestIntersect, ray, m_Segments[i]);
        }

        if (closestIntersect.z == 1000.0f)
        {
            intersectScreen(closestIntersect, ray);
        }

        m_Intersections.push_back(Vec2(closestIntersect.x, closestIntersect.y));
    }
}

//#define INTERSECTION_POINT
//#define INTERSECTION_LINE

void LightOcclusionRenderer::TriangulateMeshes()
{
    PROFILE_SCOPE("Triangulate intersections");

    const size_t numIntersections = m_Intersections.size();
    m_TriangledIntersections.resize(numIntersections*3);

#ifdef INTERSECTION_POINT
    for (size_t id = 0; id < numIntersections; id++)
    {
        Vec2 p = m_Intersections[id];
        float f = 0.1;
        m_TriangledIntersections[3*id] = p + Vec2(-0.5, -0.5) * f;
        m_TriangledIntersections[3 * id+1] = p + Vec2(0.5, -0.5) * f;
        m_TriangledIntersections[3 * id+2] = p + Vec2(0.0, 0.5) * f;
#elif defined(INTERSECTION_LINE)
    for (size_t id = 0; id < numIntersections; id++)
    {
        m_TriangledIntersections[3 * id] = m_LightSource;
        m_TriangledIntersections[3 * id + 1] = m_Intersections[id];
        m_TriangledIntersections[3 * id + 2] = m_LightSource + Vec2(0.0, 0.05);
#else
    for (size_t id = 0; id < numIntersections; id++)
    {
        size_t next_id = id + 1 == numIntersections ? 0 : id + 1;
        m_TriangledIntersections[3 * id] = m_Intersections[id];
        m_TriangledIntersections[3 * id + 1] = m_LightSource;
        m_TriangledIntersections[3 * id + 2] = m_Intersections[next_id];
#endif
    }
}

#endif