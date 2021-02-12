#include "Renderer.h"

#include "common.h"
#include "core/Window.h"
#include "gfx/GLCore.h"

#include "scene/Scene.h"
#include "scene/Entity.h"

#include "util/Profiler.h"

static std::vector<Vertex> quadVertices = 
{
    {Vec2(-1.0,-1.0)   ,Vec2(0.0,0.0)},
    {Vec2(1.0,-1.0)    ,Vec2(1.0,0.0)},
    {Vec2(-1.0,1.0)    ,Vec2(0.0,1.0)},

    {Vec2(-1.0,1.0)    ,Vec2(0.0,1.0)},
    {Vec2(1.0,-1.0)    ,Vec2(1.0,0.0)},
    {Vec2(1.0,1.0)     ,Vec2(1.0,1.0)}
};

static bool CreateShader(const std::string& name, Shader*& shader)
{
    Shader* _shader = new Shader(name + ".vert", name + ".frag");
    if (_shader->IsValid())
    {
        SAFE_DELETE(shader);
        shader = _shader;
        return true;
    }
    else if(shader)
    {
        delete _shader;
        LOG("[CREATE_SHADER] Reloading shader " + name + "failed.");
        return false;
    }
    else
    {
        delete _shader;
        ASSERT(0);
        return false;
    }
}

static bool CreateCShader(const std::string& name, ComputeShader*& shader)
{
    ComputeShader* _shader = new ComputeShader(name + ".cs");
    if (_shader->IsValid())
    {
        SAFE_DELETE(shader);
        shader = _shader;
        return true;
    }
    else if (shader)
    {
        delete _shader;
        LOG("[CREATE_SHADER] Reloading shader " + name + "failed.");
        return false;
    }
    else
    {
        delete _shader;
        ASSERT(0);
        return false;
    }
}

Renderer::~Renderer()
{
    delete m_TriangledIntersecitonsBuffer;
    delete m_TriangledIntersecitonsShaderInput;

    delete m_IntersectionShaderInput;
    delete m_IntersectionBuffer;
    
    delete m_OcclusionLines;

    delete m_TrianglulateIntersectionsShader;
    delete m_LightOcclusionShader;
    delete m_ShadowmapShader;
    delete m_OpaqueShader;

    delete m_QuadInput;
    if (m_Scene) FreeScene();
}

void Renderer::Init(Window& window)
{
    GLFunctions::InitGL(window.GetProcessAddressHandle());

    CompileShaders();

    m_QuadInput = new ShaderInput(quadVertices);
    
    static constexpr unsigned MAX_LINE_SEGMENTS = 300; // TODO : Implement UB resize and use dynamic size
    m_OcclusionLines = new UniformBuffer(sizeof(Vec4), MAX_LINE_SEGMENTS);

    m_IntersectionBuffer = new ShaderStorageBuffer(sizeof(Vec2), NUM_INTERSECTIONS);
    m_IntersectionShaderInput = m_IntersectionBuffer->AsShaderInput();

    m_TriangledIntersecitonsBuffer = new ShaderStorageBuffer(sizeof(Vec2), NUM_TRIANGLED_INTERSECTION_VERTICES);
    m_TriangledIntersecitonsShaderInput = m_TriangledIntersecitonsBuffer->AsShaderInput();
}

void Renderer::Update(float dt)
{
    // Update last render time
    static float timeUntilLastRender = 0.0f;
    timeUntilLastRender += dt;
    if (timeUntilLastRender > TICK)
    {
        m_ShouldRender = true;
        timeUntilLastRender = 0;
    }

    // Reload shaders if needed
    if (m_ShouldReloadShaders)
    {
        CompileShaders();
        m_ShouldReloadShaders = false;
    }
}

bool Renderer::RenderIfNeeded()
{
    if (m_ShouldRender)
    {
        RenderFrame();
        m_ShouldRender = false;
        return true;
    }

    return false;
}

// TODO: Apply rotation
static Mat3 GetTransformation(Transform t)
{
    return Mat3({
        t.scale.x,0.0,t.position.x,
        0.0,t.scale.y,t.position.y,
        0.0,0.0,1.0 });
}

void Renderer::RenderFrame()
{
    PROFILE_SCOPE("RenderFrame");

    GLFunctions::ClearScreen();

    static constexpr Vec2 lightPos = Vec2(-0.8, -0.1);

    {
        PROFILE_SCOPE("Light occlusion");

        // Prepare occlusion meshes
        m_OcclusionLineCount = 0;
        for (auto it = m_Scene->Begin(); it != m_Scene->End(); it++)
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

        m_LightOcclusionShader->Bind();
        m_LightOcclusionShader->SetUniform("lightPosition", lightPos);
        m_LightOcclusionShader->SetUniform("numSegments", (int) m_OcclusionLineCount);
        m_IntersectionBuffer->Bind(1);
        m_OcclusionLines->Bind(2);
        GLFunctions::Dispatch(NUM_INTERSECTIONS);
    }

    {
        PROFILE_SCOPE("Triangulate intersections");

        GLFunctions::MemoryBarrier(BarrierType::BufferUpdate); // Not sure if this is right barrier
        m_TrianglulateIntersectionsShader->Bind();
        m_TrianglulateIntersectionsShader->SetUniform("lightPosition", lightPos);
        m_IntersectionBuffer->Bind(1);
        m_TriangledIntersecitonsBuffer->Bind(2);
        GLFunctions::Dispatch(NUM_TRIANGLED_INTERSECTION_VERTICES);
    }

    {
        PROFILE_SCOPE("Opaque");

        m_OpaqueShader->Bind();
        m_QuadInput->Bind();
        m_OpaqueShader->SetUniform("u_Texture", 0);
        for (auto it = m_Scene->Begin(); it != m_Scene->End(); it++)
        {
            Entity& e = (*it);
            m_OpaqueShader->SetUniform("u_Transform", GetTransformation(e.m_Transform));
            e.m_Texture->Bind(0);
            GLFunctions::Draw(6);
        }
    }

    {
        PROFILE_SCOPE("Shadow map");

        GLFunctions::MemoryBarrier(BarrierType::VertexBuffer);
        m_ShadowmapShader->Bind();
        m_TriangledIntersecitonsShaderInput->Bind();
        GLFunctions::Draw(NUM_TRIANGLED_INTERSECTION_VERTICES);
    }
}

void Renderer::CompileShaders()
{
    CreateShader("main", m_OpaqueShader);
    CreateShader("shadowmap", m_ShadowmapShader);
    CreateCShader("light_occlusion", m_LightOcclusionShader);
    CreateCShader("triangulate_intersections", m_TrianglulateIntersectionsShader);
}

void Renderer::InitEntityForRender(Entity& e)
{
    if (!e.m_ReadyForDraw)
    {
        Texture* tex = new Texture(e.m_TexturePath);
        e.m_Texture = tex;
        e.m_Transform.scale *= Vec2((float)tex->GetWidth() / SCREEN_WIDTH, (float)tex->GetHeight() / SCREEN_HEIGHT);
        e.m_ReadyForDraw = true;
    }
}

void Renderer::RemoveEntityFromRenderPipeline(Entity& e)
{
    // TODO: Support UniformBuffer deleting m_LineSegments
    Texture* tex = e.m_Texture;
    if (tex) delete tex;
    e.m_ReadyForDraw = false;
}

void Renderer::SetScene(Scene* scene)
{
    m_Scene = scene;
    for (auto it = m_Scene->Begin(); it != m_Scene->End(); it++)
    {
        InitEntityForRender((*it));
    }
}

void Renderer::FreeScene()
{
    if (m_Scene)
    {
        for (auto it = m_Scene->Begin(); it != m_Scene->End(); it++)
        {
            RemoveEntityFromRenderPipeline((*it));
        }
        m_Scene = nullptr;
    }
}

void Renderer::OnEntityAdded(Entity& e)
{
    InitEntityForRender(e);
}

void Renderer::OnEntityRemoved(Entity& e)
{
    RemoveEntityFromRenderPipeline(e);
}