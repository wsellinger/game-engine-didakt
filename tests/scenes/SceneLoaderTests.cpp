#include "scenes/SceneLoader.h"

#include "ecs/components/BoxColliderComponent.h"
#include "ecs/components/RenderComponent.h"
#include "ecs/components/RigidBodyComponent.h"
#include "ecs/components/SpriteSheetComponent.h"
#include "ecs/components/TileMapComponent.h"
#include "ecs/components/TransformComponent.h"

#include "../mocks/TempJsonFile.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Scenes.SceneLoader.LoadFromFile_ParsesValidSceneWithAllComponents", "[SceneLoader]")
{
    TempJsonFile tempFile(R"({
        "name": "TestLevel",
        "assets": [
            { "id": "player", "path": "assets/player.png" }
        ],
        "entities": [
            {
                "transform": { "position": [10, 20], "rotation": 45, "scale": [2, 2] },
                "render": { "textureId": "player", "zIndex": 3 },
                "rigidBody": { "velocity": [1, -1] },
                "boxCollider": { "width": 16, "height": 16, "offset": [0, 0] },
                "spriteSheet": { "x": 0, "y": 0, "width": 32, "height": 32 },
                "tileMap": {
                    "tileWidth": 32,
                    "tileHeight": 32,
                    "sheetColumns": 8,
                    "tiles": [[1, 2], [3, 4]]
                }
            }
        ]
    })");

    auto result = SceneLoader::LoadFromFile(tempFile.PathString());
    REQUIRE(result.has_value());

    REQUIRE(result->scene.GetName() == "TestLevel");

    REQUIRE(result->assets.size() == 1);
    REQUIRE(result->assets[0].id == "player");
    REQUIRE(result->assets[0].path == "assets/player.png");

    auto view = result->scene.GetRegistry().view<TransformComponent, RenderComponent,
        RigidBodyComponent, BoxColliderComponent, SpriteSheetComponent, TileMapComponent>();
    REQUIRE(std::distance(view.begin(), view.end()) == 1);

    entt::entity entity = *view.begin();
    const auto& transform = result->scene.GetRegistry().get<TransformComponent>(entity);
    REQUIRE(transform.position.x == 10.f);
    REQUIRE(transform.position.y == 20.f);
    REQUIRE(transform.rotation == 45.f);

    const auto& boxCollider = result->scene.GetRegistry().get<BoxColliderComponent>(entity);
    REQUIRE(boxCollider.width == 16);
    REQUIRE(boxCollider.height == 16);

    const auto& spriteSheet = result->scene.GetRegistry().get<SpriteSheetComponent>(entity);
    REQUIRE(spriteSheet.width == 32);
    REQUIRE(spriteSheet.height == 32);

    const auto& tileMap = result->scene.GetRegistry().get<TileMapComponent>(entity);
    REQUIRE(tileMap.tileWidth == 32);
    REQUIRE(tileMap.sheetColumns == 8);
    REQUIRE(tileMap.tiles.size() == 2);
    REQUIRE(tileMap.tiles[0] == std::vector<int>{1, 2});
    REQUIRE(tileMap.tiles[1] == std::vector<int>{3, 4});
}

TEST_CASE("Scenes.SceneLoader.LoadFromFile_ParsesTileMapNestedArray", "[SceneLoader]")
{
    TempJsonFile tempFile(R"({
        "name": "TileLevel",
        "entities": [
            {
                "tileMap": {
                    "tileWidth": 32,
                    "tileHeight": 32,
                    "sheetColumns": 8,
                    "tiles": [
                        [1, 1, 2],
                        [1, 0, 2]
                    ]
                }
            }
        ]
    })");

    auto result = SceneLoader::LoadFromFile(tempFile.PathString());
    REQUIRE(result.has_value());

    auto view = result->scene.GetRegistry().view<TileMapComponent>();
    REQUIRE(std::distance(view.begin(), view.end()) == 1);

    const auto& tileMap = result->scene.GetRegistry().get<TileMapComponent>(*view.begin());
    REQUIRE(tileMap.tileWidth == 32);
    REQUIRE(tileMap.sheetColumns == 8);
    REQUIRE(tileMap.tiles.size() == 2);
    REQUIRE(tileMap.tiles[0] == std::vector<int>{1, 1, 2});
    REQUIRE(tileMap.tiles[1] == std::vector<int>{1, 0, 2});
}

TEST_CASE("Scenes.SceneLoader.LoadFromFile_PartialEntityOnlySetsPresentComponents", "[SceneLoader]")
{
    TempJsonFile tempFile(R"({
        "name": "PartialLevel",
        "entities": [
            {
                "transform": { "position": [0, 0] }
            }
        ]
    })");

    auto result = SceneLoader::LoadFromFile(tempFile.PathString());
    REQUIRE(result.has_value());

    auto& registry = result->scene.GetRegistry();
    auto view = registry.view<TransformComponent>();
    REQUIRE(std::distance(view.begin(), view.end()) == 1);

    entt::entity entity = *view.begin();
    REQUIRE_FALSE(registry.any_of<RenderComponent, RigidBodyComponent,
        BoxColliderComponent, SpriteSheetComponent, TileMapComponent>(entity));
}

TEST_CASE("Scenes.SceneLoader.LoadFromFile_MissingNameFallsBackToUnnamed", "[SceneLoader]")
{
    TempJsonFile tempFile(R"({ "entities": [] })");

    auto result = SceneLoader::LoadFromFile(tempFile.PathString());
    REQUIRE(result.has_value());
    REQUIRE(result->scene.GetName() == "Unnamed");
}

TEST_CASE("Scenes.SceneLoader.LoadFromFile_MissingAssetsAndEntitiesDefaultEmpty", "[SceneLoader]")
{
    TempJsonFile tempFile(R"({ "name": "BareLevel" })");

    auto result = SceneLoader::LoadFromFile(tempFile.PathString());
    REQUIRE(result.has_value());
    REQUIRE(result->assets.empty());

    auto view = result->scene.GetRegistry().view<entt::entity>();
    REQUIRE(view.begin() == view.end());
}

TEST_CASE("Scenes.SceneLoader.LoadFromFile_MissingFileReturnsNullopt", "[SceneLoader]")
{
    auto result = SceneLoader::LoadFromFile("this/path/does/not/exist.json");
    REQUIRE_FALSE(result.has_value());
}

TEST_CASE("Scenes.SceneLoader.LoadFromFile_MalformedJsonReturnsNullopt", "[SceneLoader]")
{
    TempJsonFile tempFile("{ this is not valid json ");

    auto result = SceneLoader::LoadFromFile(tempFile.PathString());
    REQUIRE_FALSE(result.has_value());
}