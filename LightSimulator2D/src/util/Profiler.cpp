#include "Profiler.h"

#include <chrono>

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
	float duration = std::chrono::duration<float, std::milli>(std::chrono::high_resolution_clock::now() - m_StartTime).count();
	s_CurrentState[m_Name] = duration;
}

