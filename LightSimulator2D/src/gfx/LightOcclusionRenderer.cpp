#include "LightOcclusionRenderer.h"

#include "util/Profiler.h"

#include "core/Engine.h"
#include "input/Input.h"

#include "gfx/GLCore.h"
#include "scene/Entity.h"
#include "scene/Scene.h"

LightOcclusionRenderer::LightOcclusionRenderer()
{
    if (m_UseGPU) InitGPUResources();
}

LightOcclusionRenderer::~LightOcclusionRenderer()
{
    delete m_TriangulationShader;
    delete m_OcclusionShader;
    if(m_UseGPU) DeleteGPUResources();
}

void LightOcclusionRenderer::InitGPUResources()
{
    static constexpr unsigned MAX_LINE_SEGMENTS = 300; // TODO : Implement UB resize and use dynamic size
    m_OcclusionLines = new UniformBuffer(sizeof(Vec4), MAX_LINE_SEGMENTS);

    m_IntersectionBuffer = new ShaderStorageBuffer(sizeof(Vec2), NUM_INTERSECTIONS);

    m_TriangledIntersecitonsBuffer = new ShaderStorageBuffer(sizeof(Vec2), NUM_TRIANGLED_INTERSECTION_VERTICES);
    m_TriangledIntersecitonsShaderInput = m_TriangledIntersecitonsBuffer->AsShaderInput();
}

void LightOcclusionRenderer::DeleteGPUResources()
{
    delete m_OcclusionLines;
    delete m_IntersectionBuffer;
    delete m_TriangledIntersecitonsBuffer;
    delete m_TriangledIntersecitonsShaderInput;
}

void LightOcclusionRenderer::SetUseGPU(bool value)
{
    if (value == m_UseGPU) return;

    m_UseGPU = value;
    
    if (m_UseGPU)
    {
        InitGPUResources();
    }
    else
    {
        DeleteGPUResources();
    }
}

void LightOcclusionRenderer::RenderOcclusion(Scene* scene)
{
    if (m_UseGPU)
    {
        LightOcclusionGPU(scene);
        TriangulateMeshesGPU();
    }
    else
    {
        LightOcclusionCPU(scene);
        TriangulateMeshesCPU();
    }
}

static Mat3 GetTransformation(Transform t)
{
    return Mat3({
        t.scale.x,0.0,t.position.x,
        0.0,t.scale.y,t.position.y,
        0.0,0.0,1.0 });
}

void LightOcclusionRenderer::LightOcclusionGPU(Scene* scene)
{
    PROFILE_SCOPE("Light occlusion");

    m_OcclusionLineCount = 0;
    for (auto it = scene->Begin(); it != scene->End(); it++)
    {
        Entity& e = (*it);
        static std::vector<Vec2> aabbVertices = { {-1.0,-1.0},{1.0,-1.0},{1.0,1.0},{-1.0,1.0} };
        Mat3 transform = GetTransformation(e.m_Transform);

        for (size_t i = 0; i < aabbVertices.size() - 1; i++)
        {
            Vec3 a = Vec3(aabbVertices[i], 1.0) * transform;
            Vec3 b = Vec3(aabbVertices[i + 1], 1.0) * transform;
            Vec4 lineSegment = Vec4(a.x, a.y, b.x, b.y);
            m_OcclusionLines->UploadData(&lineSegment, m_OcclusionLineCount);
            m_OcclusionLineCount++;
        }
    }

    m_OcclusionShader->Bind();
    m_OcclusionShader->SetUniform("lightPosition", Vec2(0.0, 0.0));
    m_OcclusionShader->SetUniform("numSegments", (int)m_OcclusionLineCount);
    m_IntersectionBuffer->Bind(1);
    m_OcclusionLines->Bind(2);
    GLFunctions::Dispatch(NUM_INTERSECTIONS);
}

void LightOcclusionRenderer::TriangulateMeshesGPU()
{
    PROFILE_SCOPE("Triangulate intersections");

    GLFunctions::MemoryBarrier(BarrierType::BufferUpdate); // Not sure if this is right barrier
    m_TriangulationShader->Bind();
    m_TriangulationShader->SetUniform("lightPosition", Vec2(0.0, 0.0));
    m_IntersectionBuffer->Bind(1);
    m_TriangledIntersecitonsBuffer->Bind(2);
    GLFunctions::Dispatch(NUM_TRIANGLED_INTERSECTION_VERTICES);
}

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

void LightOcclusionRenderer::LightOcclusionCPU(Scene* scene)
{
    PROFILE_SCOPE("Light occlusion");

    std::vector<Vec4> segments;
    m_OcclusionLineCount = 0;
    for (auto it = scene->Begin(); it != scene->End(); it++)
    {
        Entity& e = (*it);
        static std::vector<Vec2> aabbVertices = { {-1.0,-1.0},{1.0,-1.0},{1.0,1.0},{-1.0,1.0} };
        Mat3 transform = GetTransformation(e.m_Transform);

        for (size_t i = 0; i < aabbVertices.size(); i++)
        {
            Vec3 a = Vec3(aabbVertices[i], 1.0) * transform;
            size_t next_i = i + 1 == 4 ? 0 : i + 1;
            Vec3 b = Vec3(aabbVertices[next_i], 1.0) * transform;
            Vec4 lineSegment = Vec4(a.x, a.y, b.x, b.y);
            segments.push_back(lineSegment);
        }
    }
#ifdef MULTI_INTERSECTION
    for (size_t id = 0; id < NUM_INTERSECTIONS; id++)
    {
        float angle = id * (6.283f / NUM_INTERSECTIONS);
        float dx = cos(angle);
        float dy = sin(angle);

        Vec3 closestIntersect;
        closestIntersect.z = -1.0f;
        Vec4 ray = Vec4(m_LightPosition, m_LightPosition + Vec2(dx, dy));

        for (size_t i = 0; i < segments.size(); i++)
        {
            calcIntersection(closestIntersect, ray, segments[i]);
        }

        if (closestIntersect.x == 1000.0)
        {
            intersectScreen(closestIntersect, ray);
        }

        m_Intersections[id] = Vec2(closestIntersect.x, closestIntersect.y);
    }
#else
    Vec3 closestIntersect;
    closestIntersect.z = 1000.0f;
    Vec2 rayDirection = GameEngine::Get()->GetInput()->GetMousePosition();
    Vec4 ray = Vec4(m_LightPosition.x, m_LightPosition.y, rayDirection.x, rayDirection.y);
    for (size_t i = 0; i < segments.size(); i++)
    {
        calcIntersection(closestIntersect, ray, segments[i]);
    }
    if (closestIntersect.z == 1000.0)
    {
        intersectScreen(closestIntersect, ray);
    }
    m_Intersections[0] = Vec2(closestIntersect.x, closestIntersect.y);
#endif
}

//#define INTERSECTION_POINT
#define INTERSECTION_LINE

void LightOcclusionRenderer::TriangulateMeshesCPU()
{
    PROFILE_SCOPE("Triangulate intersections");

    SAFE_DELETE(m_TriangledIntersecitonsShaderInput);


#ifdef INTERSECTION_POINT
    std::vector<Vec2> triangledIntersections{ NUM_INTERSECTIONS * 3 };
    for (size_t id = 0; id < NUM_INTERSECTIONS; id++)
    {
        Vec2 p = m_Intersections[id];
        float f = 0.1;
        triangledIntersections[3*id] = p + Vec2(-0.5, -0.5) * f;
        triangledIntersections[3 * id+1] = p + Vec2(0.5, -0.5) * f;
        triangledIntersections[3 * id+2] = p + Vec2(0.0, 0.5) * f;
#elif defined(INTERSECTION_LINE)
    std::vector<Vec2> triangledIntersections{ NUM_INTERSECTIONS * 3 };
    for (size_t id = 0; id < NUM_INTERSECTIONS; id++)
    {
        triangledIntersections[3 * id] = m_LightPosition;
        triangledIntersections[3 * id + 1] = m_Intersections[id];
        triangledIntersections[3 * id + 2] = m_LightPosition + Vec2(0.0, 0.05);
#else
    std::vector<Vec2> triangledIntersections{ (NUM_INTERSECTIONS-1) * 3 };
    for (size_t id = 0; id < NUM_INTERSECTIONS-1; id++)
        {
        triangledIntersections[3 * id] = m_Intersections[id];
        triangledIntersections[3 * id + 1] = m_LightPosition;
        triangledIntersections[3 * id + 2] = m_Intersections[id + 1];
#endif
    }

    m_TriangledIntersecitonsShaderInput = new ShaderInput(triangledIntersections);
}