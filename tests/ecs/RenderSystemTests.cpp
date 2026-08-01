#include "ecs/systems/RenderSystem.h"
#include "ecs/components/RenderComponent.h"
#include "ecs/components/SpriteSheetComponent.h"
#include "ecs/components/TileMapComponent.h"
#include "ecs/components/TransformComponent.h"
#include "providers/interfaces/IAssetProvider.h"
#include "providers/interfaces/IRenderProvider.h"
#include "render/AssetManager.h"
#include "render/Camera.h"

#include <catch2/catch_test_macros.hpp>

#include <entt/entity/registry.hpp>

#include <unordered_map>
#include <vector>

namespace
{
    class MockAssetProvider : public IAssetProvider
    {
    public:
        TextureHandle LoadTexture(const std::string& path) override
        {
            static int nextHandleId = 1;
            void* fakeHandle = reinterpret_cast<void*>(static_cast<intptr_t>(nextHandleId++));
            TextureHandle handle{ fakeHandle };

            _handlesByPath[path] = handle;
            return handle;
        }

        void DestroyTexture(TextureHandle) override {}

        // Test helper — look up the handle that was returned for a given path,
        // so tests can assert draw-call identity rather than just distinctness.
        TextureHandle GetHandleForPath(const std::string& path) const
        {
            auto it = _handlesByPath.find(path);
            return it != _handlesByPath.end() ? it->second : TextureHandle{};
        }

    private:
        std::unordered_map<std::string, TextureHandle> _handlesByPath;
    };

    struct DrawCall
    {
        TextureHandle texture;
        RenderRect source;
        RenderRect destination;
        double rotation;
    };

    class MockRenderProvider : public IRenderProvider
    {
    public:
        void Clear(RenderColor) override { clearCallCount++; }
        void Present() override { presentCallCount++; }

        void DrawTexture(TextureHandle texture, const RenderRect& source, const RenderRect& destination, double rotation) override
        {
            drawCalls.push_back({ texture, source, destination, rotation });
        }

        int clearCallCount = 0;
        int presentCallCount = 0;
        std::vector<DrawCall> drawCalls;
    };
}

TEST_CASE("ECS.RenderSystem.SpriteEntity_DrawsOnce", "[RenderSystem]")
{
    entt::registry registry;
    MockAssetProvider assetProvider;
    AssetManager assetManager;
    assetManager.Initialize(assetProvider);
    assetManager.LoadTexture("player", "assets/player.png");

    auto entity = registry.create();
    registry.emplace<RenderComponent>(entity, RenderComponent{ "player", 0 });
    registry.emplace<SpriteSheetComponent>(entity, SpriteSheetComponent{ 0, 0, 32, 32 });
    registry.emplace<TransformComponent>(entity, TransformComponent{ glm::vec2{ 100.0f, 50.0f }, 0.0f, glm::vec2{ 1.0f, 1.0f } });

    MockRenderProvider renderProvider;
    Camera camera{ 0.0f, 0.0f, 1.0f };
    RenderSystem renderSystem;

    renderSystem.Render(registry, renderProvider, assetManager, camera);

    REQUIRE(renderProvider.drawCalls.size() == 1);
    REQUIRE(renderProvider.drawCalls[0].destination.x == 100);
    REQUIRE(renderProvider.drawCalls[0].destination.y == 50);
    REQUIRE(renderProvider.drawCalls[0].destination.w == 32);
    REQUIRE(renderProvider.drawCalls[0].destination.h == 32);
}

TEST_CASE("ECS.RenderSystem.SpriteEntity_PassesRotation", "[RenderSystem]")
{
    entt::registry registry;
    MockAssetProvider assetProvider;
    AssetManager assetManager;
    assetManager.Initialize(assetProvider);
    assetManager.LoadTexture("player", "assets/player.png");

    auto entity = registry.create();
    registry.emplace<RenderComponent>(entity, RenderComponent{ "player", 0 });
    registry.emplace<SpriteSheetComponent>(entity, SpriteSheetComponent{ 0, 0, 32, 32 });
    registry.emplace<TransformComponent>(entity, TransformComponent{ glm::vec2{ 0.0f, 0.0f }, 45.0, glm::vec2{ 1.0f, 1.0f } });

    MockRenderProvider renderProvider;
    Camera camera{ 0.0f, 0.0f, 1.0f };
    RenderSystem renderSystem;

    renderSystem.Render(registry, renderProvider, assetManager, camera);

    REQUIRE(renderProvider.drawCalls.size() == 1);
    REQUIRE(renderProvider.drawCalls[0].rotation == 45.0);
}

TEST_CASE("ECS.RenderSystem.TileMapEntity_DrawsOncePerTile", "[RenderSystem]")
{
    entt::registry registry;
    MockAssetProvider assetProvider;
    AssetManager assetManager;
    assetManager.Initialize(assetProvider);
    assetManager.LoadTexture("tileset", "assets/tileset.png");

    auto entity = registry.create();
    registry.emplace<RenderComponent>(entity, RenderComponent{ "tileset", 0 });
    registry.emplace<TileMapComponent>(entity, TileMapComponent
        {
            {
                { 0, 1 },
                { 2, 3 }
            },
            16, 16, 4
        });

    MockRenderProvider renderProvider;
    Camera camera{ 0.0f, 0.0f, 1.0f };
    RenderSystem renderSystem;

    renderSystem.Render(registry, renderProvider, assetManager, camera);

    REQUIRE(renderProvider.drawCalls.size() == 4); // 2x2 grid
}

TEST_CASE("ECS.RenderSystem.TileMapEntity_TilesHaveZeroRotation", "[RenderSystem]")
{
    entt::registry registry;
    MockAssetProvider assetProvider;
    AssetManager assetManager;
    assetManager.Initialize(assetProvider);
    assetManager.LoadTexture("tileset", "assets/tileset.png");

    auto entity = registry.create();
    registry.emplace<RenderComponent>(entity, RenderComponent{ "tileset", 0 });
    registry.emplace<TileMapComponent>(entity, TileMapComponent
        {
            { { 0 } },
            16, 16, 4
        });

    MockRenderProvider renderProvider;
    Camera camera{ 0.0f, 0.0f, 1.0f };
    RenderSystem renderSystem;

    renderSystem.Render(registry, renderProvider, assetManager, camera);

    REQUIRE(renderProvider.drawCalls.size() == 1);
    REQUIRE(renderProvider.drawCalls[0].rotation == 0.0);
}

TEST_CASE("ECS.RenderSystem.DrawOrderRespectsZIndex", "[RenderSystem]")
{
    entt::registry registry;
    MockAssetProvider assetProvider;
    AssetManager assetManager;
    assetManager.Initialize(assetProvider);
    assetManager.LoadTexture("background", "assets/background.png");
    assetManager.LoadTexture("foreground", "assets/foreground.png");

    auto back = registry.create();
    registry.emplace<RenderComponent>(back, RenderComponent{ "background", 10 });
    registry.emplace<SpriteSheetComponent>(back, SpriteSheetComponent{ 0, 0, 16, 16 });
    registry.emplace<TransformComponent>(back, TransformComponent{ glm::vec2{ 0.0f, 0.0f }, 0.0f, glm::vec2{ 1.0f, 1.0f } });

    auto front = registry.create();
    registry.emplace<RenderComponent>(front, RenderComponent{ "foreground", 1 });
    registry.emplace<SpriteSheetComponent>(front, SpriteSheetComponent{ 0, 0, 16, 16 });
    registry.emplace<TransformComponent>(front, TransformComponent{ glm::vec2{ 0.0f, 0.0f }, 0.0f, glm::vec2{ 1.0f, 1.0f } });

    MockRenderProvider renderProvider;
    Camera camera{ 0.0f, 0.0f, 1.0f };
    RenderSystem renderSystem;

    renderSystem.Render(registry, renderProvider, assetManager, camera);

    REQUIRE(renderProvider.drawCalls.size() == 2);

    TextureHandle backgroundHandle = assetProvider.GetHandleForPath("assets/background.png");
    TextureHandle foregroundHandle = assetProvider.GetHandleForPath("assets/foreground.png");

    // Lower zIndex (foreground, 1) must draw before higher zIndex (background, 10)
    REQUIRE(renderProvider.drawCalls[0].texture.handle == foregroundHandle.handle);
    REQUIRE(renderProvider.drawCalls[1].texture.handle == backgroundHandle.handle);
}

TEST_CASE("ECS.RenderSystem.NoEntities_ClearAndPresentStillCalledByEngine", "[RenderSystem]")
{
    // RenderSystem::Render itself doesn't call Clear/Present (Engine does,
    // around the call) — confirm Render produces zero draw calls with an
    // empty registry, and doesn't crash.
    entt::registry registry;
    MockAssetProvider assetProvider;
    AssetManager assetManager;
    assetManager.Initialize(assetProvider);

    MockRenderProvider renderProvider;
    Camera camera{ 0.0f, 0.0f, 1.0f };
    RenderSystem renderSystem;

    renderSystem.Render(registry, renderProvider, assetManager, camera);

    REQUIRE(renderProvider.drawCalls.empty());
    REQUIRE(renderProvider.clearCallCount == 0);
    REQUIRE(renderProvider.presentCallCount == 0);
}