#include "core/Engine.h"

#include "util/StringUtil.h"

#include <time.h>

int main(int argc, char* argv[])
{
    srand((unsigned)time(NULL));

    int sceneIndex = 0;
    if (argc > 1) StringUtil::ParseInt(argv[1], sceneIndex);

    GameEngine* engine = GameEngine::Get();
    engine->Init(sceneIndex);
    engine->Run();
    GameEngine::Delete();

    return 0;
}