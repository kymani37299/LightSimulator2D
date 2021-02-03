#include "Renderer.h"

#include "common.h"
#include "core/Window.h"
#include "gfx/GLCore.h"

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
    delete m_Texture;
}

void Renderer::Init(Window& window)
{
    GLFunctions::InitGL(window.GetProcessAddressHandle());
    m_Shader = new Shader("main.vert", "main.frag");
    ASSERT(m_Shader->IsValid());

    m_QuadInput = new ShaderInput(quadVertices);
    m_Texture = new Texture("res/Grass.jpg");
}

void Renderer::Update(float dt)
{
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

void Renderer::RenderFrame()
{
    m_Shader->Bind();
    m_QuadInput->Bind();

    m_Texture->Bind(0);
    m_Shader->SetUniform("u_Texture", 0);

    GLFunctions::Draw(6);
}