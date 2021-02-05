#include "Renderer.h"

#include "common.h"
#include "core/Window.h"
#include "gfx/GLCore.h"

#include "scene/Scene.h"
#include "scene/Entity.h"

static std::vector<Vertex> quadVertices = 
{
    {Vec2(-1.0,-1.0)   ,Vec2(0.0,0.0)},
    {Vec2(1.0,-1.0)    ,Vec2(1.0,0.0)},
    {Vec2(-1.0,1.0)    ,Vec2(0.0,1.0)},

    {Vec2(-1.0,1.0)    ,Vec2(0.0,1.0)},
    {Vec2(1.0,-1.0)    ,Vec2(1.0,0.0)},
    {Vec2(1.0,1.0)     ,Vec2(1.0,1.0)}
};

Renderer::~Renderer()
{
    // TMP START - Compute shaders test
    delete m_ComputeShader;
    delete m_Image;
    // TMP END - Compute shaders test

    delete m_Framebuffer; // TMP - Framebuffer test

    delete m_Shader;
    delete m_QuadInput;
    if (m_Scene) FreeScene();
}

void Renderer::Init(Window& window)
{
    GLFunctions::InitGL(window.GetProcessAddressHandle());
    m_Shader = new Shader("main.vert", "main.frag");
    ASSERT(m_Shader->IsValid());

    m_QuadInput = new ShaderInput(quadVertices);

    // TMP START - Compute shaders test
    m_ComputeShader = new ComputeShader("test.cs");
    m_Image = new Image(512, 512, IF_WriteAccess);
    // TMP END - Compute shaders test

    m_Framebuffer = new Framebuffer(SCREEN_WIDTH, SCREEN_HEIGHT); // TMP - Framebuffer test
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
    GLFunctions::ClearScreen();

    // TMP START - Framebuffer test
    m_Framebuffer->Bind();
    GLFunctions::ClearScreen();
    // TMP END - Framebuffer test

    m_Shader->Bind();
    m_QuadInput->Bind();
    m_Shader->SetUniform("u_Texture", 0);
    for (auto it = m_Scene->Begin(); it != m_Scene->End(); it++)
    {
        m_Shader->SetUniform("u_Transform", GetTransformation((*it).m_Transform));
        (*it).m_Texture->Bind(0);
        GLFunctions::Draw(6);
    }

    // TMP START - Framebuffer test
    m_Framebuffer->Unbind();
    m_Framebuffer->BindTexture(0, 0);
    m_Shader->SetUniform("u_Transform", GetTransformation({ Vec2(-0.5,0.5),Vec2(0.5,0.5), 0 }));
    GLFunctions::Draw(6);
    // TMP END - Framebuffer test

    // TMP START - Compute shaders test
    m_ComputeShader->Bind();
    m_Image->Bind(0);
    GLFunctions::Dispatch(512, 512);
    GLFunctions::MemoryBarrier(BarrierType::Image);
    m_Shader->Bind();
    m_QuadInput->Bind();
    m_Shader->SetUniform("u_Transform", GetTransformation({ Vec2(0.2,0.2),Vec2(0.2,0.2), 0 }));
    GLFunctions::Draw(6);
    // TMP END - Compute shaders test

}

void Renderer::InitEntityForRender(Entity& e)
{
    if (!e.m_ReadyForDraw)
    {
        Texture* tex = new Texture(e.m_TexturePath);
        e.m_Texture = tex;
        e.m_Transform.scale *= Vec2((float)tex->GetWidth() / SCREEN_WIDTH, (float)tex->GetHeight() / SCREEN_HEIGHT);
        e.m_ReadyForDraw = true;

        if (e.m_DrawFlags.emitter)
        {
            m_PointLights.push_back({ e.m_EntityID, e.m_Transform.position, e.m_EmissionColor });
        }
    }
}

void Renderer::RemoveEntityFromRenderPipeline(Entity& e)
{
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