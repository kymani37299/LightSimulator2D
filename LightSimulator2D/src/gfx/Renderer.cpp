#include "Renderer.h"

#include <glad/glad.h>

#include "common.h"
#include "core/Window.h"


Renderer::~Renderer()
{

}

void Renderer::Init(Window& window)
{
    int glad_status = gladLoadGLLoader((GLADloadproc)window.GetProcessAddressHandle());
    ASSERT(glad_status);
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

}