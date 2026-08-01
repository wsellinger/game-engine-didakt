#include "SceneManager.h"

#include "../core/Logger.h"

bool SceneManager::LoadScene(const std::string& path, AssetManager& assetManager)
{
    std::optional<SceneLoadResult> result = SceneLoader::LoadFromFile(path);
    if (!result)
    {
        Logger::Log(LogLevel::Error, "SceneManager: Failed to load scene from file: %s", path.c_str());
        return false;
    }

    assetManager.ClearAll();
    
    for (const auto& asset : result->assets)
    {
        assetManager.LoadTexture(asset.id, asset.path);
    }

    _currentScene = std::move(result->scene);
    
    return true;
}

Scene& SceneManager::GetCurrentScene()
{
    DIDAKT_ASSERT(_currentScene.has_value());
    return *_currentScene;
}
