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
    delete m_Shader;
    delete m_QuadInput;

    for (Texture* tex : m_Textures)
    {
        delete tex;
    }
}

void Renderer::Init(Window& window)
{
    GLFunctions::InitGL(window.GetProcessAddressHandle());
    m_Shader = new Shader("main.vert", "main.frag");
    ASSERT(m_Shader->IsValid());

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

    // Update scene
    if (m_Scene->IsDirty())
    {
        for (auto it = m_Scene->Begin(); it != m_Scene->End(); it++)
        {
            Entity& e = (*it);
            if (!e.m_ReadyForDraw)
            {
                Texture* tex = new Texture(e.m_TexturePath);
                m_Textures.push_back(tex);
                e.m_Texture = tex;
                e.m_Transform.scale *= Vec2( (float) tex->GetWidth() / SCREEN_WIDTH, (float) tex->GetHeight() / SCREEN_HEIGHT);
                e.m_ReadyForDraw = true;
            }
        }
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
    m_Shader->Bind();
    m_QuadInput->Bind();
    m_Shader->SetUniform("u_Texture", 0);
    for (auto it = m_Scene->Begin(); it != m_Scene->End(); it++)
    {
        m_Shader->SetUniform("u_Transform", GetTransformation((*it).m_Transform));
        (*it).m_Texture->Bind(0);
        GLFunctions::Draw(6);
    }
}