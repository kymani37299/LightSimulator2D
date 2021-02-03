#pragma once

#include <string>

namespace StringUtil
{
    static void ReplaceAll(std::string& str, const std::string& from, const std::string& to) {
        if (from.empty())
            return;
        size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length();
        }
    }

    static bool ParseFloat(const char* str, float& value)
    {
        try 
        {
            std::string stdStr = std::string(str);
            value = std::stof(stdStr);
        }
        catch (const std::exception&) { return false; }
        return true;
    }

    static std::string GetPathWitoutFile(std::string path)
    {
        return path.substr(0, 1 + path.find_last_of("\\/"));
    }
}
