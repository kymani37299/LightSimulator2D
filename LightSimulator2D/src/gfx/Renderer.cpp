#include "Renderer.h"

#include "common.h"
#include "core/Window.h"
#include "gfx/GLCore.h"
#include "gfx/LightOcclusionRenderer.h"

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
    delete m_OcclusionRenderer;
    delete m_ShadowmapShader;
    delete m_OpaqueShader;

    delete m_QuadInput;
    if (m_Scene) FreeScene();
}

void Renderer::Init(Window& window)
{
    GLFunctions::InitGL(window.GetProcessAddressHandle());

    m_OcclusionRenderer = new LightOcclusionRenderer();

    CompileShaders();

    m_QuadInput = new ShaderInput(quadVertices);
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

    m_OcclusionRenderer->RenderOcclusion(m_Scene);

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
        ShaderInput* occlusionInput = m_OcclusionRenderer->GetOcclusionMesh();
        occlusionInput->Bind();
        GLFunctions::Draw(occlusionInput->GetElementNumber());
    }
}

void Renderer::CompileShaders()
{
    CreateShader("main", m_OpaqueShader);
    CreateShader("shadowmap", m_ShadowmapShader);
    CreateCShader("light_occlusion", m_OcclusionRenderer->GetOcclusionShader());
    CreateCShader("triangulate_intersections", m_OcclusionRenderer->GetTriangulateShader());
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