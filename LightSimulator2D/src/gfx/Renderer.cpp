#include "Renderer.h"

#include "common.h"

#include "core/Window.h"

#include "gfx/GLCore.h"
#include "gfx/LightOcclusionRenderer.h"
#include "gfx/LightingRenderer.h"

#include "scene/Scene.h"
#include "scene/Entity.h"

#include "util/Profiler.h"

static std::vector<Vertex> quadVertices = 
{
    {Vec2(-1.0,-1.0)   ,Vec2(0.0,0.0)},
    {Vec2(1.0,-1.0)    ,Vec2(1.0,0.0)},
    {Vec2(-1.0,1.0)    ,Vec2(0.0,1.0)},

    {Vec2(-1.0,1.0)    ,Vec2(0.0,1.0)},
    {Vec2(1.0,-1.0)    ,Vec2(1.0,0.0)},
    {Vec2(1.0,1.0)     ,Vec2(1.0,1.0)}
};

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
    delete m_AlbedoFB;

    delete m_OcclusionRenderer;
    delete m_LightingRenderer;

    delete m_AlbedoShader;

    if (m_Scene) FreeScene();

    GLFunctions::DeinitGL();
}

void Renderer::Init(Window& window)
{
    GLFunctions::InitGL(window.GetProcessAddressHandle());
    m_AlbedoFB = new Framebuffer(SCREEN_WIDTH, SCREEN_HEIGHT);

    m_OcclusionRenderer = new LightOcclusionRenderer();
    m_LightingRenderer = new LightingRenderer(m_AlbedoFB, m_OcclusionRenderer->GetOcclusionMaskFB());

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

    m_OcclusionRenderer->Update(dt);

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

    GLFunctions::ClearScreen();
    m_OcclusionRenderer->RenderOcclusion(m_Scene);
    RenderAlbedo();
    m_LightingRenderer->RenderLights(m_Scene);
}

inline void RenderEntity(Shader* shader, Entity* e)
{
    if (!e->GetDrawFlags().background) shader->SetUniform("u_Transform", e->GetTransformation());

    e->GetTexture()->Bind(0);

    Texture* normal = e->GetNormalMap();
    if (normal) normal->Bind(1);
    shader->SetUniform("u_NormalEnabled", normal != nullptr);

    GLFunctions::DrawPoints(1);
}

void Renderer::RenderAlbedo()
{
    Camera& cam = m_Scene->GetCamera();

    PROFILE_SCOPE("Albedo");
    m_AlbedoFB->ClearAndBind();
    m_AlbedoShader->Bind();

    // TODO: No emitters and multiple emitters
    Vec2 lightSource = m_Scene->GetEmitters()[0]->m_Transform.position + cam.position;
    m_AlbedoShader->SetUniform("u_LightSource", lightSource);

    m_AlbedoShader->SetUniform("u_View", MAT3_IDENTITY);
    m_AlbedoShader->SetUniform("u_Transform", MAT3_IDENTITY);

    // Render background
    Entity* bg = m_Scene->GetBackground();
    m_AlbedoShader->SetUniform("u_UVScale", bg->GetBackgroundProperties().textureScale / cam.zoom);
    m_AlbedoShader->SetUniform("u_UVOffset", -(cam.position + bg->m_Transform.position)/2.0f);
    if (bg) RenderEntity(m_AlbedoShader, bg);

    m_AlbedoShader->SetUniform("u_View", cam.GetTransformation());
    m_AlbedoShader->SetUniform("u_UVScale", 1.0f);
    m_AlbedoShader->SetUniform("u_UVOffset", VEC2_ZERO);

    for (auto it = m_Scene->Begin(); it != m_Scene->End(); it++)
    {
        Entity* e = (*it);
        if (e == bg) continue;

        RenderEntity(m_AlbedoShader, e);
    }
    m_AlbedoFB->Unbind();
}

void Renderer::CompileShaders()
{
    CreateShader("albedo", m_AlbedoShader);
    m_OcclusionRenderer->CompileShaders();
    m_LightingRenderer->CompileShaders();
}

void Renderer::InitEntityForRender(Entity* e)
{
    if (e->m_ReadyForDraw) return;

    Texture* tex = new Texture(e->m_TexturePath);
    e->m_Texture = tex;
    e->m_Transform.scale *= Vec2((float)tex->GetWidth() / SCREEN_WIDTH, (float)tex->GetHeight() / SCREEN_HEIGHT);
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