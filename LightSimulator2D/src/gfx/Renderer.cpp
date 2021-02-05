#include "Renderer.h"

#include "common.h"
#include "core/Window.h"
#include "gfx/GLCore.h"

#include "scene/Scene.h"
#include "scene/Entity.h"

static std::vector<Vertex> quadVertices = 
{
    {Vec2(-1.0,-1.0)   ,Vec2(0.0,0.0)},
    {Vec2(1.0,-1.0)    ,Vec2(1.0,0.0)},
    {Vec2(-1.0,1.0)    ,Vec2(0.0,1.0)},

    {Vec2(-1.0,1.0)    ,Vec2(0.0,1.0)},
    {Vec2(1.0,-1.0)    ,Vec2(1.0,0.0)},
    {Vec2(1.0,1.0)     ,Vec2(1.0,1.0)}
};

static constexpr unsigned MAX_BATCH_SIZE = 100;

struct EntityInstanceData
{
    // Mat3 dont have good alignment because it is 4*vec4 row based
    alignas(16) Vec3 transformation_row_1;
    alignas(16) Vec3 transformation_row_2;
    alignas(16) Vec3 transformation_row_3;
    static unsigned GetStride() { return 12 * 4; }
};

Renderer::~Renderer()
{
    delete m_StaticTransformBuffer;
    delete m_Shader;
    delete m_QuadInput;
    if (m_Scene) FreeScene();
}

void Renderer::Init(Window& window)
{
    GLFunctions::InitGL(window.GetProcessAddressHandle());
    m_Shader = new Shader("main.vert", "main.frag");
    ASSERT(m_Shader->IsValid());

    m_QuadInput = new ShaderInput(quadVertices);
    m_StaticTransformBuffer = new UniformBuffer(EntityInstanceData::GetStride(), MAX_BATCH_SIZE);
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

// TODO: Apply rotation
static Mat3 GetTransformation(Transform t)
{
    return Mat3({
        t.scale.x,0.0,t.position.x,
        0.0,t.scale.y,t.position.y,
        0.0,0.0,1.0 });
}

static EntityInstanceData GetInstanceData(Transform t)
{
    Mat3 transformation = GetTransformation(t);
    EntityInstanceData instanceData;
    instanceData.transformation_row_1 = transformation[0];
    instanceData.transformation_row_2 = transformation[1];
    instanceData.transformation_row_3 = transformation[2];
    return instanceData;
}

void Renderer::RenderFrame()
{
    GLFunctions::ClearScreen();

    int staticIndex = 0;

    m_Shader->Bind();
    m_QuadInput->Bind();
    m_Shader->SetUniform("u_Texture", 0);
    m_StaticTransformBuffer->Bind(1);
    m_Shader->SetUniformBlock("StaticBuffer", 1);
    for (auto it = m_Scene->Begin(); it != m_Scene->End(); it++)
    {
        Entity& e = (*it);
        if (e.m_Static)
        {
            m_Shader->SetUniform("u_StaticIndex", staticIndex);
            staticIndex++;
        }
        else
        {
            m_Shader->SetUniform("u_StaticIndex", -1);
            m_Shader->SetUniform("u_Transform", GetTransformation(e.m_Transform));
        }
        e.m_Texture->Bind(0);
        GLFunctions::Draw(6);
    }
}

void Renderer::InitEntityForRender(Entity& e)
{
    if (!e.m_ReadyForDraw)
    {
        Texture* tex = new Texture(e.m_TexturePath);
        e.m_Texture = tex;
        e.m_Transform.scale *= Vec2((float)tex->GetWidth() / SCREEN_WIDTH, (float)tex->GetHeight() / SCREEN_HEIGHT);
        e.m_ReadyForDraw = true;

        if (e.m_DrawFlags.emitter)
        {
            m_PointLights.push_back({ e.m_EntityID, e.m_Transform.position, e.m_EmissionColor });
        }

        if (e.m_Static)
        {
            EntityInstanceData instanceData = GetInstanceData(e.m_Transform);
            m_StaticTransformBuffer->UploadData(&instanceData, m_NumStaticTransforms, 1);
            m_NumStaticTransforms++;
        }
    }
}

void Renderer::RemoveEntityFromRenderPipeline(Entity& e)
{
    // TODO: Not supported dynamic removing because it will make static transform buffer out of sync
    Texture* tex = e.m_Texture;
    if (tex) delete tex;
    e.m_ReadyForDraw = false;
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

void Renderer::OnEntityAdded(Entity& e)
{
    InitEntityForRender(e);
}

void Renderer::OnEntityRemoved(Entity& e)
{
    RemoveEntityFromRenderPipeline(e);
}