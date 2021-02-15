#include "FpsProfiler.h"

#include "core/Engine.h"
#include "core/Window.h"
#include <imgui.h>

static constexpr float FPS_UPDATE_INTERVAL = 1000.0f; // in ms
static constexpr size_t MAX_FPS_VALUES = 500;

FpsProfiler::FpsProfiler()
{
	m_Window = GameEngine::Get()->GetMainWindow();
}

void FpsProfiler::Update(float dt)
{
	m_FpsAge += dt;
	if (m_FpsAge > FPS_UPDATE_INTERVAL)
	{
		m_FpsAge -= FPS_UPDATE_INTERVAL;
		m_CurrentFps = m_Window->GetFps();
		m_FpsValues.push_back(m_Window->GetFps());
		if (m_FpsValues.size() > MAX_FPS_VALUES)
		{
			m_FpsValues.clear();
		}
	}
}

void FpsProfiler::Render()
{
	IM_BEGIN("Fps profiler");
	ImGui::Text("Current FPS: %3.f", m_CurrentFps);
	ImGui::PlotLines("FPS: ", m_FpsValues.data(), m_FpsValues.size());
	IM_END();
}