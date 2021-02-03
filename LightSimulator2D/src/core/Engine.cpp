#include "Engine.h"

#include "common.h"
#include <chrono>

#include "input/Controller.h"
#include "scene/Entity.h"

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
    m_Window.SetInput(&m_Input);
    m_Renderer.Init(m_Window);
    m_Controller = new PlayerController();
    m_Controller->Init(&m_Input);
    m_Renderer.SetScene(&m_Scene);

    // TMP
    Entity e1{ "res/animals/elephant.png" };
    Entity e2{ "res/animals/hippo.png" };
    e2.m_Transform.scale *= 0.2;
    e2.m_Transform.position = Vec2(-0.3, 0.5);
    m_Scene.AddEntity(e1);
    m_Scene.AddEntity(e2);
    //
}

void GameEngine::EngineLoop()
{
    m_Running = m_Window.Active();
    UpdateDT();
    m_Controller->Update(m_DT);
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