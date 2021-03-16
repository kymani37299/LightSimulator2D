#include <chrono>

#include "core/Engine.h"

int main(int argc, char* argv[])
{
    int sceneIndex = 0;

    if (argc > 1)
    {
        try 
        {
            sceneIndex = std::stoi(argv[1]);
        }
        catch (std::exception const& e) 
        { 
            (void)e;
            sceneIndex = 0;
        }
    }


    GameEngine* engine = GameEngine::Get();
    engine->Init(sceneIndex);
    engine->Run();
    GameEngine::Delete();

    return 0;
}