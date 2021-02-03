#include "Renderer.h"

#include "common.h"
#include "core/Window.h"
#include "gfx/GLCore.h"

Renderer::~Renderer()
{

}

void Renderer::Init(Window& window)
{
    GLFunctions::InitGL(window.GetProcessAddressHandle());
    m_Shader = new Shader("main.vert", "main.frag");
    ASSERT(m_Shader->IsValid());

    std::vector<Vertex> vertices = {
        {Vec2(-1.0,-1.0),Vec2(0.0,0.0)},
        {Vec2(0.0,1.0),Vec2(0.5,1.0)},
        {Vec2(1.0,-1.0),Vec2(1.0,0.0)}
    };
    m_Triangle = new ShaderInput(vertices);
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
    m_Triangle->Bind();

    m_Texture->Bind(0);
    m_Shader->SetUniform("u_Texture", 0);

    GLFunctions::Draw(3);
}