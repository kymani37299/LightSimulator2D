#pragma once

#include <string>

#define LOG(X) Logger::Get()->ConsoleLog(X);

class Logger
{
private:
	static Logger* s_Instance;

public:
	static Logger* Get();

public:
	void ConsoleLog(const std::string& message);
};