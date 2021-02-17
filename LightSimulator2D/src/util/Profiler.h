#pragma once

#include <map>
#include <string>
#include <chrono>

#define PROFILE_SCOPE(X) Profiler JOIN(scopedProfiler,__LINE__){X}
#define PROFILE_GET(X) Profiler::GetTime(X)

class ProfilerUI;

using ProfilerState = std::map<std::string, float>;

class Profiler
{
	friend class ProfilerUI;

	static ProfilerState s_CurrentState; // times in ms

public:
	Profiler(const std::string& name);
	~Profiler();

	static float GetTime(const std::string& name);

private:
	std::string m_Name;
	std::chrono::time_point<std::chrono::steady_clock> m_StartTime;
};