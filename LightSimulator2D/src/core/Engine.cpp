#include "Engine.h"

#include "common.h"
#include <chrono>

#include "input/Controller.h"
#include "util/Profiler.h"
#include "scene/DemoScene.h"

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
    }
    return s_Instance;
}

void GameEngine::Delete()
{
    SAFE_DELETE(s_Instance);
}

GameEngine::~GameEngine()
{
    delete m_Controller;
}

void GameEngine::Init(unsigned demoIndex)
{
    PlayerControllerComponent* controllerComponent = Demo::SetupDemoScene(&m_Scene, demoIndex);

    m_Window.SetInput(&m_Input);
    m_Renderer.Init(m_Window);
    m_UI.Init(&m_Window);
    m_Scene.Init(&m_Renderer);

    m_Controller = new PlayerController(controllerComponent);
    m_Controller->Init(&m_Input);
}

void GameEngine::EngineLoop()
{
    PROFILE_SCOPE("EngineLoop");

    m_Running = m_Window.Active();
    UpdateDT();
    m_Controller->Update(m_DT);
    m_Scene.Update(m_DT);
    m_UI.Update(m_DT);
    m_Renderer.Update(m_DT);
    if (m_Renderer.RenderIfNeeded())
    {
        m_UI.Render();
        m_Window.UpdateGraphic();
    }
    m_Window.Update(m_DT);
}

void GameEngine::Run()
{
    m_Running = true;
    while (m_Running)
    {
        Profiler::BeginFrame();
        EngineLoop();
        Profiler::EndFrame();
    }
}

void GameEngine::UpdateDT()
{
    static auto t_before = std::chrono::high_resolution_clock::now();
    auto t_now = std::chrono::high_resolution_clock::now();
    m_DT = std::chrono::duration<float, std::milli>(t_now - t_before).count();
    t_before = t_now;
}