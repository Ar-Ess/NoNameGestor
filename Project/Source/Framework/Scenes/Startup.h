#pragma once

using StartupSceneFunction = bool(*)();

void SetStartupSceneFunction(StartupSceneFunction function);
StartupSceneFunction GetStartupSceneFunction();

class SceneManager;

#include "Framework/Scenes/SceneManager.h"

template<class T>
bool StartupScene()
{
    SceneManager::PushScene<T>();
    return SceneManager::ChangeScene<T>();
}

#define REGISTER_STARTUP_SCENE(SceneType) \
namespace \
{ \
    struct RegisterStartupScene \
    { \
        RegisterStartupScene() \
        { \
            SetStartupSceneFunction(&StartupScene<SceneType>); \
        } \
    }; \
    static RegisterStartupScene registerStartupScene; \
}
