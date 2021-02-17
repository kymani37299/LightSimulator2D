#include "UIEngine.h"

#include "imgui.h"
#include "ImGUI_impl.h"

#include "core/Window.h"

#include "elements/ProfilerUI.h"

void UIEngine::SetupElements()
{
    AddElement(new ProfilerUI());
}

UIEngine::~UIEngine()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void UIEngine::Init(Window* window)
{
	IMGUI_CHECKVERSION();
    
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    
    ImGui::StyleColorsDark();
    
    ImGui_ImplGlfw_InitForOpenGL(window->GetWindowHandle(), true);
    ImGui_ImplOpenGL3_Init("#version 330");

    SetupElements();
}

void UIEngine::Update(float dt)
{
    for (size_t i=0;i<m_Elements.size();i++)
    {
        UIElement* e = m_Elements[i];

        if (e->IsVisible()) e->Update(dt);
        else
        {
            RemoveElement(i);
            i--;
        }
    }
}

void UIEngine::Render()
{
    BeginFrame();
    for (UIElement* e : m_Elements) e->Render();
    EndFrame();
}

void UIEngine::BeginFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void UIEngine::EndFrame()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UIEngine::AddElement(UIElement* e)
{
    e->Init();
    m_Elements.push_back(e);
    e->Show();
}

void UIEngine::RemoveElement(size_t index)
{
    UIElement* e = m_Elements[index];
    m_Elements.erase((m_Elements.begin() + index));
    delete e;
}