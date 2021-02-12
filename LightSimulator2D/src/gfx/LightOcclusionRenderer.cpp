#include "LightOcclusionRenderer.h"

#include "util/Profiler.h"

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
    // Parametric form : Origin + t*Direction
    // ro + t1*rd
    Vec2 ro = Vec2(ray.x, ray.y);
    Vec2 rd = Vec2(ray.z, ray.w);

    // a + t2*b
    Vec2 a = Vec2(segment.x, segment.y);
    Vec2 b = Vec2(segment.z, segment.w);

    float t2_1 = rd.x * (a.y - ro.y) + rd.y * (ro.x - a.x);
    float t2_2 = b.x * rd.y - b.y * rd.x;
    float t2 = t2_1 / t2_2;

    float t1_1 = a.x + b.x * t2 - ro.x;
    float t1_2 = rd.x;
    float t1 = t1_1 / t1_2;

    // Parallel lines
    if (t1_2 == 0 || t2_2 == 0) return;

    // Intersect test
    if (t1 > 0.0 && t2 > 0.0 && t2 < 1.0 && (intersection.z < t1 || intersection.z == -1))
    {
        Vec2 p = ro + t1 * rd;
        intersection.x = p.x;
        intersection.y = p.y;
        intersection.z = t1;
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

        for (size_t i = 0; i < aabbVertices.size() - 1; i++)
        {
            Vec3 a = Vec3(aabbVertices[i], 1.0) * transform;
            Vec3 b = Vec3(aabbVertices[i + 1], 1.0) * transform;
            Vec4 lineSegment = Vec4(a.x, a.y, b.x, b.y);
            segments.push_back(lineSegment);
        }
    }

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
}

void LightOcclusionRenderer::TriangulateMeshesCPU()
{
    PROFILE_SCOPE("Triangulate intersections");

    SAFE_DELETE(m_TriangledIntersecitonsShaderInput);

    std::vector<Vec2> triangledIntersections{ NUM_INTERSECTIONS * 3 };
    for (size_t id = 0; id < NUM_INTERSECTIONS - 1; id++)
    {
        //Vec2 p = intersections[id];
        //float f = 0.1;
        //triangledIntersections[3*id] = p + Vec2(-0.5, -0.5) * f;
        //triangledIntersections[3 * id+1] = p + Vec2(0.5, -0.5) * f;
        //triangledIntersections[3 * id+2] = p + Vec2(0.0, 0.5) * f;

        triangledIntersections[3 * id] = m_Intersections[id];
        triangledIntersections[3 * id + 1] = m_LightPosition;
        triangledIntersections[3 * id + 2] = m_Intersections[id + 1];
    }

    m_TriangledIntersecitonsShaderInput = new ShaderInput(triangledIntersections);
}