#include "ProfilerUI.h"

#include "util/Profiler.h"
#include "core/Window.h"

#include <imgui.h>
#include <algorithm>

static const std::string FToS(float v)
{
	return std::to_string(v).substr(0, 4);
}

static bool nameComparator(const ProfilerDiagram* l, const ProfilerDiagram* r) 
{ 
	return l->GetLabel().compare(r->GetLabel()) < 0; 
}

static bool timeComparator(const ProfilerDiagram* l, const ProfilerDiagram* r)
{
	return l->GetCurrentMS() > r->GetCurrentMS();
}

void ProfilerDiagram::AddValue(float value)
{
	if (m_Values.size() >= MAX_VALUES)
	{
		m_Values.clear();
	}
	m_Values.push_back(value);
	m_CurrentValue = value;
}

void ProfilerDiagram::Render()
{
	ImGui::Text((m_Label + ":" + FToS(m_CurrentValue) + " (ms)").c_str());
	ImGui::PlotLines("", m_Values.data(), m_Values.size());
}

ProfilerUI::~ProfilerUI()
{
	for (ProfilerDiagram* pd : m_Diagrams)
	{
		delete pd;
	}
	m_Diagrams.clear();
}

void ProfilerUI::Update(float dt)
{
	m_LastUpdateAge += dt;
	if (m_LastUpdateAge > UPDATE_INTERVAL)
	{
		UpdateInternal();
		m_LastUpdateAge = 0;
	}
}

void ProfilerUI::UpdateInternal()
{
	ProfilerState& profilerState = Profiler::GetCurrentState();
	auto it = profilerState.begin();
	bool needSort = !m_SortByName || m_SortByName != m_SortByNameCB;
	m_SortByName = m_SortByNameCB;
	while (it != profilerState.end())
	{		
		if (std::find_if(m_Diagrams.begin(), m_Diagrams.end(), [&](ProfilerDiagram* pd) { return it->first == pd->GetLabel(); }) == m_Diagrams.end())
		{
			m_Diagrams.push_back(new ProfilerDiagram(it->first));
			needSort = true;
		}
		it++;
	}

	for (ProfilerDiagram* pd : m_Diagrams)
	{
		pd->AddValue(Profiler::GetTime(pd->GetLabel()));
	}

	if (needSort) std::sort(m_Diagrams.begin(), m_Diagrams.end(), m_SortByName ? &nameComparator : &timeComparator);

	m_CurrentFPS = Profiler::GetFPS();
}

void ProfilerUI::Render()
{
	IM_BEGIN("Profiler");
#ifdef DEBUG
	ImGui::Checkbox("Sort by name:", &m_SortByNameCB);
#endif
	ImGui::Text(("FPS: " + FToS(m_CurrentFPS)).c_str());

#ifdef DEBUG // TODO: Hide diagrams until is fixed bug with release
	for (ProfilerDiagram* d : m_Diagrams)
	{
		d->Render();
	}
#endif
	IM_END();
}