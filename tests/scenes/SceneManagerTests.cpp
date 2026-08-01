#include "scenes/SceneManager.h"

#include "../mocks/MockAssetProvider.h"
#include "../mocks/TempJsonFile.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Scenes.SceneManager.LoadScene_ValidFileSetsCurrentScene", "[SceneManager]")
{
    TempJsonFile tempFile(R"({
        "name": "Level1",
        "entities": [ { "transform": { "position": [0, 0] } } ]
    })", "didakt_test_scenemanager_1.json");

    MockAssetProvider provider;
    AssetManager assetManager;
    assetManager.Initialize(provider);

    SceneManager sceneManager;
    bool loaded = sceneManager.LoadScene(tempFile.PathString(), assetManager);

    REQUIRE(loaded);
    REQUIRE(sceneManager.GetCurrentScene().GetName() == "Level1");
}

TEST_CASE("Scenes.SceneManager.LoadScene_LoadsSceneAssetsIntoAssetManager", "[SceneManager]")
{
    TempJsonFile tempFile(R"({
        "name": "Level1",
        "assets": [ { "id": "player", "path": "assets/player.png" } ]
    })", "didakt_test_scenemanager_2.json");

    MockAssetProvider provider;
    AssetManager assetManager;
    assetManager.Initialize(provider);

    SceneManager sceneManager;
    sceneManager.LoadScene(tempFile.PathString(), assetManager);

    REQUIRE(provider.loadCallCount == 1);
    REQUIRE(provider.lastLoadedPath == "assets/player.png");
}

TEST_CASE("Scenes.SceneManager.LoadScene_ClearsPreviousAssetsBeforeLoadingNew", "[SceneManager]")
{
    TempJsonFile firstFile(R"({
        "name": "Level1",
        "assets": [ { "id": "player", "path": "assets/player.png" } ]
    })", "didakt_test_scenemanager_3a.json");

    TempJsonFile secondFile(R"({
        "name": "Level2",
        "assets": [ { "id": "enemy", "path": "assets/enemy.png" } ]
    })", "didakt_test_scenemanager_3b.json");

    MockAssetProvider provider;
    AssetManager assetManager;
    assetManager.Initialize(provider);

    SceneManager sceneManager;
    sceneManager.LoadScene(firstFile.PathString(), assetManager);
    sceneManager.LoadScene(secondFile.PathString(), assetManager);

    REQUIRE(provider.destroyCallCount == 1); // Level1's "player" cleared
    REQUIRE(provider.loadCallCount == 2);     // player, then enemy
}

TEST_CASE("Scenes.SceneManager.LoadScene_MissingFileReturnsFalse", "[SceneManager]")
{
    MockAssetProvider provider;
    AssetManager assetManager;
    assetManager.Initialize(provider);

    SceneManager sceneManager;
    bool loaded = sceneManager.LoadScene("this/path/does/not/exist.json", assetManager);

    REQUIRE_FALSE(loaded);
    REQUIRE(provider.loadCallCount == 0);
    REQUIRE(provider.destroyCallCount == 0);
}

TEST_CASE("Scenes.SceneManager.LoadScene_FailedLoadDoesNotReplaceCurrentScene", "[SceneManager]")
{
    TempJsonFile validFile(R"({ "name": "First" })", "didakt_test_scenemanager_5.json");

    MockAssetProvider provider;
    AssetManager assetManager;
    assetManager.Initialize(provider);

    SceneManager sceneManager;
    sceneManager.LoadScene(validFile.PathString(), assetManager);

    bool secondLoad = sceneManager.LoadScene("this/path/does/not/exist.json", assetManager);

    REQUIRE_FALSE(secondLoad);
    REQUIRE(sceneManager.GetCurrentScene().GetName() == "First");
}