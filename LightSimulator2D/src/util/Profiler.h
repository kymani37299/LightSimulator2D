#pragma once

#include <map>
#include <string>
#include <chrono>

#ifdef DEBUG
#define FORCE_BARRIERS
#endif

#define PROFILE_SCOPE(X) Profiler JOIN(scopedProfiler,__LINE__){X}

class ProfilerUI;

using ProfilerState = std::map<std::string, float>;

class Profiler
{
	friend class ProfilerUI;

	static ProfilerState s_CurrentStatePP1;	// times in ms
	static ProfilerState s_CurrentStatePP2;	// times in ms
	static bool s_PP;

	static std::chrono::time_point<std::chrono::steady_clock> s_FrameBeginTime;
	static float s_FPS;

public:
	Profiler(const std::string& name);
	~Profiler();

	static float GetTime(const std::string& name);
	static float GetFPS();
	static void BeginFrame();
	static void EndFrame();
	static ProfilerState& GetCurrentState() { return s_PP ? s_CurrentStatePP1 : s_CurrentStatePP2; }
	static ProfilerState& GetOtherState() { return s_PP ? s_CurrentStatePP2 : s_CurrentStatePP1; }

private:
	std::string m_Name;
	std::chrono::time_point<std::chrono::steady_clock> m_StartTime;
};