#include "Scene.h"

#include <optional>
#include <string>
#include <vector>

struct AssetEntry
{
    std::string id;
    std::string path;
};

struct SceneLoadResult
{
    Scene scene;
    std::vector<AssetEntry> assets;
};

namespace SceneLoader
{
    std::optional<SceneLoadResult> LoadFromFile(const std::string& path);
}