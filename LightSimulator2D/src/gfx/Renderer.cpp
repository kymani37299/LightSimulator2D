#include "Renderer.h"

#include "common.h"
#include "core/Window.h"
#include "gfx/GLCore.h"

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

Renderer::~Renderer()
{
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
    
    static constexpr unsigned MAX_LINE_SEGMENTS = 300; // TODO : Implement UB resize and use dynamic size
    m_OcclusionLines = new UniformBuffer(sizeof(Vec4), MAX_LINE_SEGMENTS);
    m_IntersectionBuffer = new ShaderStorageBuffer(sizeof(Vec2), NUM_INTERSECTIONS);

    m_LightOcclusionShader = new ComputeShader("light_occlusion.cs");
    ASSERT(m_LightOcclusionShader->IsValid());
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

void Renderer::RenderFrame()
{
    PROFILE_SCOPE("RenderFrame");

    GLFunctions::ClearScreen();

    {
        PROFILE_SCOPE("Light occlusion");

        m_LightOcclusionShader->Bind();
        m_LightOcclusionShader->SetUniform("lightPosition", Vec2(-0.8, -0.1));
        m_LightOcclusionShader->SetUniform("numSegments", (int) m_OcclusionLineCount);
        m_IntersectionBuffer->Bind(1);
        m_OcclusionLines->Bind(2);
        GLFunctions::Dispatch(NUM_INTERSECTIONS);
    }

    {
        PROFILE_SCOPE("OpaquePass");

        m_Shader->Bind();
        m_QuadInput->Bind();
        m_Shader->SetUniform("u_Texture", 0);
        for (auto it = m_Scene->Begin(); it != m_Scene->End(); it++)
        {
            Entity& e = (*it);
            m_Shader->SetUniform("u_Transform", GetTransformation(e.m_Transform));
            e.m_Texture->Bind(0);
            GLFunctions::Draw(6);
        }
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

        // TODO: Do this for dynamic meshes
        static std::vector<Vec2> aabbVertices = {{-1.0,-1.0},{1.0,-1.0},{1.0,1.0},{-1.0,1.0}};
        Mat3 transform = GetTransformation(e.m_Transform);

        for (size_t i = 0; i < aabbVertices.size() - 1; i++)
        {
            Vec3 a = Vec3(aabbVertices[i],1.0) * transform;
            Vec3 b = Vec3(aabbVertices[i+1],1.0) * transform;
            Vec4 lineSegment = Vec4(a.x,a.y,b.x, b.y);
            m_OcclusionLines->UploadData(&lineSegment, m_OcclusionLineCount);
            m_OcclusionLineCount++;
        }
    }
}

void Renderer::RemoveEntityFromRenderPipeline(Entity& e)
{
    // TODO: Support UniformBuffer deleting m_LineSegments
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