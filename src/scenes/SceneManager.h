#pragma once

#include "Scene.h"
#include "SceneLoader.h"

#include "../render/AssetManager.h"
#include "../core/Assert.h"

#include <optional>
#include <string>

class SceneManager
{
public:
	SceneManager() = default;

	bool LoadScene(const std::string& path, AssetManager& assetManager);
    Scene& GetCurrentScene();

private:
    std::optional<Scene> _currentScene;
};