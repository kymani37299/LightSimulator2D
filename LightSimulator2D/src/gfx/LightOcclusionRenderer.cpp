#include "LightOcclusionRenderer.h"

#include "util/Profiler.h"

#include "core/Engine.h"
#include "input/Input.h"

#include "gfx/GLCore.h"
#include "scene/Entity.h"

extern bool CreateShader(const std::string& name, Shader*& shader);
extern bool CreateCShader(const std::string& name, ComputeShader*& shader);

LightOcclusionRenderer::LightOcclusionRenderer()
{
#ifdef GPU_OCCLUSION
    m_OcclusionLines = new UniformBuffer(sizeof(Vec4), MAX_LINE_SEGMENTS);
    m_RayQueryBuffer = new UniformBuffer(sizeof(Vec4), MAX_RAY_QUERIES);

    m_IntersectionBuffer = new ShaderStorageBuffer(sizeof(Vec4), MAX_RAY_QUERIES);
    m_TriangledIntersecitonsBuffer = new ShaderStorageBuffer(sizeof(Vec4), MAX_RAY_QUERIES * 3);

    m_OcclusionMesh = m_TriangledIntersecitonsBuffer->AsShaderInput();
#endif

    m_OcclusionMaskFB = new Framebuffer(SCREEN_WIDTH, SCREEN_HEIGHT);

    m_OcclusionMeshOutput = new ShaderStorageBuffer(sizeof(Vec4), OCCLUSION_MESH_SIZE / 2);
}

LightOcclusionRenderer::~LightOcclusionRenderer()
{
    delete m_OcclusionMaskFB;
    delete m_OcclusionMeshGenShader;
    delete m_OcclusionMeshOutput;
    delete m_ShadowmapShader;

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

void LightOcclusionRenderer::CompileShaders()
{
#ifdef GPU_OCCLUSION
    CreateCShader("light_occlusion", m_OcclusionShader);
    CreateCShader("triangulate_intersections", m_TriangulationShader);
#endif
    CreateCShader("occlusion_mesh_gen", m_OcclusionMeshGenShader);
    CreateShader("shadowmap",m_ShadowmapShader);
}

void LightOcclusionRenderer::OnOccluderAdded(Entity& e)
{
    ASSERT(e.GetDrawFlags().occluder);
    {
        m_OcclusionMeshGenShader->Bind();
        e.m_Texture->Bind(0);
        m_OcclusionMeshOutput->Bind(1);
        GLFunctions::Dispatch(OCCLUSION_MESH_SIZE / 2);
    }

    GLFunctions::MemoryBarrier(BarrierType::ShaderStorage);
    OcclusionMesh& mesh = m_OcclusionMeshPool[&e];
    PopulateOcclusionMesh(mesh);
}

void LightOcclusionRenderer::OnOccluderRemoved(Entity& e)
{
    ASSERT(e.GetDrawFlags().occluder);
    m_OcclusionMeshPool[&e].clear();
}

void LightOcclusionRenderer::RenderOcclusion()
{
    PROFILE_SCOPE("Render occlusion");

    Vec2 mousePos = GameEngine::Get()->GetInput()->GetMousePosition();
    float angleStep = 2.0f * 3.1415f / NUM_LIGHT_SAMPLES;

    m_OcclusionMaskFB->Clear();

    for (int i = 0; i < NUM_LIGHT_SAMPLES; i++)
    {
        float angle = i * angleStep;
        m_LightSource = mousePos + Vec2(cos(angle), sin(angle)) * m_LightRadius;

        LightOcclusion();
        TriangulateMeshes();
        RenderOcclusionMask();
    }
}

void LightOcclusionRenderer::BindOcclusionMask(unsigned slot)
{
    m_OcclusionMaskFB->BindTexture(0,slot);
}

unsigned LightOcclusionRenderer::SetupOcclusionMeshInput()
{
#ifdef GPU_OCCLUSION
    unsigned numIntersections = m_RayCount*3;
#else
    SAFE_DELETE(m_OcclusionMesh);
    unsigned numIntersections = m_TriangledIntersections.size();
    m_OcclusionMesh = new ShaderInput(m_TriangledIntersections);
#endif
    m_OcclusionMesh->Bind();

    return numIntersections;
}

void LightOcclusionRenderer::SetupLineSegments()
{
    m_OcclusionLineCount = 0;
    for (auto it = m_OcclusionMeshPool.begin(); it != m_OcclusionMeshPool.end(); it++)
    {
        const Entity* e = it->first;
        const Mat3 transform = e->GetTransformation();
        const OcclusionMesh& mesh = it->second;
        for (size_t i = 0; i < mesh.size(); i++)
        {
            Vec3 a = Vec3(mesh[i], 1.0) * transform;
            size_t next_i = i + 1 == mesh.size() ? 0 : i + 1;
            Vec3 b = Vec3(mesh[next_i], 1.0) * transform;
            Vec4 lineSegment = Vec4(a.x, a.y, b.x, b.y);

#ifdef GPU_OCCLUSION
            m_OcclusionLines->UploadData(&lineSegment, m_OcclusionLineCount);
#else
            m_Segments.push_back(lineSegment);
#endif
            m_OcclusionLineCount++;
        }
    }
}

void LightOcclusionRenderer::RenderOcclusionMask()
{
    PROFILE_SCOPE("Occlusion mask");
    m_OcclusionMaskFB->Bind();
    m_ShadowmapShader->Bind();
    unsigned numVertices = SetupOcclusionMeshInput();
    GLFunctions::AlphaBlending(true);
    m_ShadowmapShader->SetUniform("u_MaskStrength", m_MaskStrength);
    GLFunctions::MemoryBarrier(BarrierType::VertexBuffer);
    GLFunctions::Draw(numVertices);
    GLFunctions::AlphaBlending(false);
    m_OcclusionMaskFB->Unbind();
}

#ifdef GPU_OCCLUSION
void LightOcclusionRenderer::SetupRayQuery()
{
    for (size_t i = 0; i < NUM_ANGLED_RAYS; i++)
    {
        float angle = i * (2.0f * 6.283f / NUM_ANGLED_RAYS);
        Vec2 dir = Vec2(cos(angle), sin(angle));
        m_RayQuery.push(dir);
    }

    const Vec2 e = Vec2(0.01f);
    for (auto it = m_OcclusionMeshPool.begin(); it != m_OcclusionMeshPool.end(); it++)
    {
        OcclusionMesh& mesh = it->second;
        for (Vec2 p : mesh)
        {
            m_RayQuery.push(p - e);
            m_RayQuery.push(e);
            m_RayQuery.push(p + e);
        }
    }

    // TODO: Optimize this, upload all at same time
    size_t rayIndex = 0;
    m_RayCount = m_RayQuery.size();
    while (!m_RayQuery.empty())
    {
        Vec2 ray = m_RayQuery.top();
        m_RayQuery.pop();
        m_RayQueryBuffer->UploadData(&ray, rayIndex);
        rayIndex++;
    }
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

void LightOcclusionRenderer::PopulateOcclusionMesh(OcclusionMesh& mesh)
{
    mesh.clear();
    std::vector<Vec2> vertices[4]; // Left, top, right, down

    int oBufferSize = OCCLUSION_MESH_SIZE / 2;
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

void LightOcclusionRenderer::LightOcclusion()
{
    PROFILE_SCOPE("Light occlusion");

    SetupLineSegments();
    SetupRayQuery();

    m_OcclusionShader->Bind();
    m_OcclusionShader->SetUniform("lightPosition", m_LightSource);
    m_OcclusionShader->SetUniform("numSegments", (int) m_OcclusionLineCount);
    m_IntersectionBuffer->Bind(1);
    m_OcclusionLines->Bind(2);
    m_RayQueryBuffer->Bind(3);
    GLFunctions::Dispatch(m_RayCount);
}

void LightOcclusionRenderer::TriangulateMeshes()
{
    PROFILE_SCOPE("Triangulate intersections");

    GLFunctions::MemoryBarrier(BarrierType::ShaderStorage);
    m_TriangulationShader->Bind();
    m_TriangulationShader->SetUniform("lightPosition", m_LightSource);
    m_TriangulationShader->SetUniform("numIntersections", (int) m_RayCount);
    m_IntersectionBuffer->Bind(1);
    m_TriangledIntersecitonsBuffer->Bind(2);
    GLFunctions::Dispatch(m_RayCount*3);
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

    for (size_t i = 0; i < NUM_ANGLED_RAYS; i++)
    {
        float angle = i * (2.0f * 6.283f / NUM_ANGLED_RAYS);
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

void LightOcclusionRenderer::LightOcclusion()
{
    PROFILE_SCOPE("Light occlusion");

    m_Segments.clear();
    SetupLineSegments();
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