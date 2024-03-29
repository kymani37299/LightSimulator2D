#include "Renderer.h"

#include "common.h"

#include "core/Window.h"

#include "gfx/GLCore.h"
#include "gfx/LightOcclusionRenderer.h"
#include "gfx/LightingRenderer.h"
#include "gfx/AlbedoRenderer.h"
#include "gfx/SceneCuller.h"
#include "gfx/DebugRenderer.h"

#include "scene/Scene.h"
#include "scene/Entity.h"

#include "util/Profiler.h"

bool CreateShader(const std::string& name, Shader*& shader)
{
    Shader* _shader = new Shader(name + ".glsl");
    if (_shader->IsValid())
    {
        SAFE_DELETE(shader);
        shader = _shader;
        return true;
    }
    else if(shader)
    {
        delete _shader;
        LOG("[CREATE_SHADER] Reloading shader " + name + "failed.");
        return false;
    }
    else
    {
        delete _shader;
        ASSERT(0);
        return false;
    }
}

Renderer::~Renderer()
{
    delete m_AlbedoRenderer;
    delete m_OcclusionRenderer;
    delete m_LightingRenderer;
    delete m_SceneCuller;
    DebugRenderer::Delete();

    if (m_Scene) FreeScene();

    GLFunctions::DeinitGL();
}

void Renderer::Init(Window& window)
{
    GLFunctions::InitGL(window.GetProcessAddressHandle());

    m_OcclusionRenderer = new LightOcclusionRenderer();
    m_AlbedoRenderer = new AlbedoRenderer();
    m_LightingRenderer = new LightingRenderer(m_AlbedoRenderer->GetAlbedoFB(), m_OcclusionRenderer->GetOcclusionMaskFB());
    m_SceneCuller = new SceneCuller();

    CompileShaders();
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

    // Reload shaders if needed
    if (m_ShouldReloadShaders)
    {
        CompileShaders();
        m_ShouldReloadShaders = false;
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
    PROFILE_SCOPE("RenderFrame");

    CulledScene& culledScene = m_SceneCuller->GetCulledScene(m_Scene);

    GLFunctions::ClearScreen();

    m_OcclusionRenderer->   RenderOcclusion     (culledScene);
    m_AlbedoRenderer->      RenderBackground    (culledScene);
    m_AlbedoRenderer->      RenderBase          (culledScene);
    m_LightingRenderer->    RenderLighting      (culledScene);
    m_AlbedoRenderer->      RenderOccluders     (culledScene);
    m_LightingRenderer->    RenderEmitters      (culledScene);
    m_AlbedoRenderer->      RenderForeground    (culledScene);
    DebugRenderer::Get()->  RenderDebug         (culledScene);
}

void Renderer::CompileShaders()
{
    m_AlbedoRenderer->CompileShaders();
    m_OcclusionRenderer->CompileShaders();
    m_LightingRenderer->CompileShaders();
    DebugRenderer::Get()->CompileShaders();
}

void Renderer::InitEntityForRender(Entity* e)
{
    if (e->m_ReadyForDraw) return;

    Texture* tex = new Texture(e->m_TexturePath);
    e->m_Texture = tex;
    Vec2 texScale = Vec2((float)tex->GetWidth() / SCREEN_WIDTH, (float)tex->GetHeight() / SCREEN_HEIGHT);
    for (EntityInstance* et : e->GetInstances())
    {
        et->ApplyScale(texScale);
    }
    e->m_ReadyForDraw = true;

    if (!e->m_NormalMapPath.empty())
    {
        e->m_NormalMap = new Texture(e->m_NormalMapPath);
    }

    if (e->GetDrawFlags().background)
    {
        e->m_Texture->SetRepeatedScaling(true);
        if (e->m_NormalMap) e->m_NormalMap->SetRepeatedScaling(true);
    }

    if(e->GetDrawFlags().occluder) m_OcclusionRenderer->OnOccluderAdded(e);
}

void Renderer::RemoveEntityFromRenderPipeline(Entity* e)
{
    if (!e->m_ReadyForDraw) return;

    SAFE_DELETE(e->m_Texture);
    SAFE_DELETE(e->m_NormalMap);
    e->m_ReadyForDraw = false;
    if (e->GetDrawFlags().occluder) m_OcclusionRenderer->OnOccluderRemoved(e);
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

void Renderer::OnEntityAdded(Entity* e)
{
    InitEntityForRender(e);
}

void Renderer::OnEntityRemoved(Entity* e)
{
    RemoveEntityFromRenderPipeline(e);
}