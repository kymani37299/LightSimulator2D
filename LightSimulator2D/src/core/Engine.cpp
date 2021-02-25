#include "Engine.h"

#include "common.h"
#include <chrono>

#include "input/Controller.h"
#include "scene/Entity.h"

#include "util/Profiler.h"

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
    // TMP
    Entity* bg = new Entity{ "res/bg.png" };
    bg->m_Transform.scale *= 1000.0f;
    Entity* e1 = new Entity{ "res/animals/elephant.png" };
    e1->GetDrawFlags().occluder = true;
    Entity* e2 = new Entity{ "res/animals/hippo.png" };
    e2->GetDrawFlags().occluder = true;
    e2->m_Transform.scale *= 0.2;
    e2->m_Transform.position = Vec2(-0.3, 0.5);
    m_Scene.AddEntity(bg);
    m_Scene.AddEntity(e1);
    m_Scene.AddEntity(e2);
    //

    m_Window.SetInput(&m_Input);
    m_Renderer.Init(m_Window);
    m_UI.Init(&m_Window);
    m_Scene.Init(&m_Renderer);

    m_Controller = new PlayerController(e1);
    m_Controller->Init(&m_Input);
}

void GameEngine::EngineLoop()
{
    PROFILE_SCOPE("EngineLoop");

    m_Running = m_Window.Active();
    UpdateDT();
    m_Controller->Update(m_DT);
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
    while (m_Running) EngineLoop();
}

void GameEngine::UpdateDT()
{
    static auto t_before = std::chrono::high_resolution_clock::now();
    auto t_now = std::chrono::high_resolution_clock::now();
    m_DT = std::chrono::duration<float, std::milli>(t_now - t_before).count();
    t_before = t_now;
}