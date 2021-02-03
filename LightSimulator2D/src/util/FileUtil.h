#pragma once

#include <string>
#include <fstream>
#include <vector>

namespace FileUtil
{
	static bool ReadFile(const std::string& path, std::vector<std::string>& content)
	{
		std::ifstream fileStream(path, std::ios::in);

		if (!fileStream.is_open()) {
			return false;
		}

		std::string line = "";
		while (!fileStream.eof()) {
			std::getline(fileStream, line);
			content.push_back(line);
		}

		fileStream.close();
		return true;
	}
}