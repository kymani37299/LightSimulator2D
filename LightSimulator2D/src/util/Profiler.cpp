#include "Profiler.h"

#include <chrono>

#ifdef FORCE_BARRIERS
#include "gfx/GLCore.h"
#endif

ProfilerState Profiler::s_CurrentStatePP1;
ProfilerState Profiler::s_CurrentStatePP2;
bool Profiler::s_PP = false;

std::chrono::time_point<std::chrono::steady_clock> Profiler::s_FrameBeginTime;
float Profiler::s_FPS = 0.0f;

float Profiler::GetTime(const std::string& name)
{
	ProfilerState& state = GetCurrentState();
	auto res = state.find(name);
	if (res == state.end()) return -1.0f;
	return res->second;
}

void Profiler::BeginFrame()
{
	s_FrameBeginTime = std::chrono::high_resolution_clock::now();
	GetCurrentState().clear();
	s_PP = !s_PP;
}

void Profiler::EndFrame()
{
	float frameDuration = std::chrono::duration<float, std::milli>(std::chrono::high_resolution_clock::now() - s_FrameBeginTime).count();
	s_FPS = 1000.0f / frameDuration;
}

Profiler::Profiler(const std::string& name):
	m_Name(name),
	m_StartTime(std::chrono::high_resolution_clock::now())
{

}

Profiler::~Profiler()
{
#ifdef FORCE_BARRIERS
	GLFunctions::WaitForGpu();
#endif
	ProfilerState& state = GetOtherState();

	float duration = std::chrono::duration<float, std::milli>(std::chrono::high_resolution_clock::now() - m_StartTime).count();
	if (state.find(m_Name) != state.end())
	{
		state[m_Name] += duration;
	}
	else
	{
		state[m_Name] = duration;
	}
	
}

