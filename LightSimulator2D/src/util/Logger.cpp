#include "Logger.h"

#include <iostream>

Logger* Logger::s_Instance = nullptr;

Logger* Logger::Get()
{
	if (!s_Instance)
		s_Instance = new Logger();
	return s_Instance;
}

void Logger::ConsoleLog(const std::string& msg)
{
	std::cout << msg << std::endl;
}