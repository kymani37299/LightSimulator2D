#include "Profiler.h"

#include <chrono>

#include "common.h"

#ifdef FORCE_BARRIERS
#include "gfx/GLCore.h"
#endif

ProfilerState Profiler::s_CurrentState;

float Profiler::GetTime(const std::string& name)
{
	auto res = s_CurrentState.find(name);
	if (res == s_CurrentState.end()) return -1.0f;
	return res->second;
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
	float duration = std::chrono::duration<float, std::milli>(std::chrono::high_resolution_clock::now() - m_StartTime).count();
	s_CurrentState[m_Name] = duration;
}

