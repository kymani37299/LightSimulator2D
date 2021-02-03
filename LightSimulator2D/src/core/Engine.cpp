#include "Engine.h"

#include "common.h"
#include <chrono>

void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    GameEngine::Get()->GetMainWindow()->KeyPressedCallback(window, key, scancode, action, mods);
}

GameEngine* GameEngine::s_Instance = nullptr;

GameEngine* GameEngine::Get()
{
    if (!s_Instance)
    {
        s_Instance = new GameEngine();
        s_Instance->Init();
    }
    return s_Instance;
}

void GameEngine::Delete()
{
    SAFE_DELETE(s_Instance);
}

GameEngine::~GameEngine()
{
}

void GameEngine::Init()
{
    m_Renderer.Init(m_Window);
}

void GameEngine::EngineLoop()
{
    m_Running = m_Window.Active();
    UpdateDT();
    m_Renderer.Update(m_DT);
    if (m_Renderer.RenderIfNeeded())
    {
        m_Window.UpdateGraphic();
    }
    m_Window.Update(m_DT);
}

void GameEngine::Run()
{
    m_Running = true;
    while (m_Running) EngineLoop();
}

void GameEngine::UpdateDT()
{
    static auto t_before = std::chrono::high_resolution_clock::now();
    auto t_now = std::chrono::high_resolution_clock::now();
    m_DT = std::chrono::duration<float, std::milli>(t_now - t_before).count();
    t_before = t_now;
}