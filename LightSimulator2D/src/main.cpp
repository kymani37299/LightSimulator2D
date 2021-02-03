#include <chrono>

#include "core/Engine.h"

int main(void)
{
    GameEngine::Get()->Run();
    GameEngine::Delete();
    return 0;
}