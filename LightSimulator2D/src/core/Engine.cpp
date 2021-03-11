#include "Engine.h"

#include "common.h"
#include <chrono>

#include "input/Controller.h"
#include "scene/Entity.h"

#include "scene/components/PlayerControllerComponent.h"
#include "scene/components/FollowMouseComponent.h"

#include "util/Profiler.h"

void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    GameEngine::Get()->GetMainWindow()->KeyPressedCallback(window, key, scancode, action, mods);
}

static void SetupTestScene(Scene* scene, PlayerControllerComponent* playerController)
{
    Entity* bg = new Entity{ "res/bg.png" };
    bg->m_Transform.scale *= 1000.0f;

    Entity* e1 = new Entity{ "res/animals/elephant.png" , "res/animals/elephant_normal.jpg" };
    e1->GetDrawFlags().occluder = true;
    e1->GetOcclusionProperties().shape = OccluderShape::Mesh;
    e1->GetOcclusionProperties().meshLod = 2;
    e1->m_Transform.rotation = 3.1415f / 4.0f;
    e1->AddComponent(playerController);

    Entity* e2 = new Entity{ "res/animals/hippo.png" };
    e2->GetDrawFlags().occluder = true;
    e2->m_Transform.scale *= 0.2;
    e2->m_Transform.position = Vec2(-0.3, 0.5);

    Entity* e3 = new Entity{ "res/animals/giraffe.png" };
    e3->AddComponent(new FollowMouseComponent());
    e3->m_Transform.scale *= 0.3;
    e3->GetDrawFlags().emitter = true;
    e3->GetEmissionProperties().color = Vec3(1.0, 1.0, 0.0);
    e3->GetEmissionProperties().radius = 0.1f;

    Entity* e4 = new Entity{ "res/animals/elephant.png" , "res/animals/elephant_normal.jpg" };
    e4->m_Transform.scale *= 0.8;
    e4->m_Transform.position = Vec2(0.8, 0.8);

    scene->AddEntity(bg);
    scene->AddEntity(e1);
    scene->AddEntity(e2);
    scene->AddEntity(e3);
    scene->AddEntity(e4);
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
    delete m_Controller;
}

void GameEngine::Init()
{
    PlayerControllerComponent* controllerComponent = new PlayerControllerComponent();
    SetupTestScene(&m_Scene, controllerComponent);

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
    while (m_Running) EngineLoop();
}

void GameEngine::UpdateDT()
{
    static auto t_before = std::chrono::high_resolution_clock::now();
    auto t_now = std::chrono::high_resolution_clock::now();
    m_DT = std::chrono::duration<float, std::milli>(t_now - t_before).count();
    t_before = t_now;
}