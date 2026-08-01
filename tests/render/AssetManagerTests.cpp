#include "render/AssetManager.h"
#include "../mocks/MockAssetProvider.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Render.AssetManager.LoadTexture_CachesById", "[AssetManager]")
{
    MockAssetProvider provider;
    AssetManager assetManager;
    assetManager.Initialize(provider);

    assetManager.LoadTexture("player", "assets/player.png");

    REQUIRE(provider.loadCallCount == 1);
    REQUIRE(provider.lastLoadedPath == "assets/player.png");
}

TEST_CASE("Render.AssetManager.LoadTexture_SameIdTwiceDoesNotReload", "[AssetManager]")
{
    MockAssetProvider provider;
    AssetManager assetManager;
    assetManager.Initialize(provider);

    assetManager.LoadTexture("player", "assets/player.png");
    assetManager.LoadTexture("player", "assets/player.png"); // same id again

    REQUIRE(provider.loadCallCount == 1); // provider only asked once
}

TEST_CASE("Render.AssetManager.LoadTexture_DifferentIdsBothLoad", "[AssetManager]")
{
    MockAssetProvider provider;
    AssetManager assetManager;
    assetManager.Initialize(provider);

    assetManager.LoadTexture("player", "assets/player.png");
    assetManager.LoadTexture("enemy", "assets/enemy.png");

    REQUIRE(provider.loadCallCount == 2);
}

TEST_CASE("Render.AssetManager.LoadTexture_FailedLoadIsNotCached", "[AssetManager]")
{
    MockAssetProvider provider;
    provider.shouldFailLoad = true;

    AssetManager assetManager;
    assetManager.Initialize(provider);

    assetManager.LoadTexture("player", "assets/missing.png");

    TextureHandle handle = assetManager.GetTextureHandle("player");
    REQUIRE_FALSE(handle.IsValid());
}

TEST_CASE("Render.AssetManager.LoadTexture_FailedLoadCanBeRetried", "[AssetManager]")
{
    // A failed load must not poison the cache — a later successful
    // load under the same id should still take effect.
    MockAssetProvider provider;
    provider.shouldFailLoad = true;

    AssetManager assetManager;
    assetManager.Initialize(provider);

    assetManager.LoadTexture("player", "assets/missing.png");
    REQUIRE_FALSE(assetManager.GetTextureHandle("player").IsValid());

    provider.shouldFailLoad = false;
    assetManager.LoadTexture("player", "assets/player.png");

    REQUIRE(assetManager.GetTextureHandle("player").IsValid());
    REQUIRE(provider.loadCallCount == 2); // both attempts actually reached the provider
}

TEST_CASE("Render.AssetManager.GetTextureHandle_UnknownIdReturnsInvalidHandle", "[AssetManager]")
{
    MockAssetProvider provider;
    AssetManager assetManager;
    assetManager.Initialize(provider);

    TextureHandle handle = assetManager.GetTextureHandle("nonexistent");

    REQUIRE_FALSE(handle.IsValid());
}

TEST_CASE("Render.AssetManager.GetTextureHandle_ReturnsSameHandleAcrossCalls", "[AssetManager]")
{
    MockAssetProvider provider;
    AssetManager assetManager;
    assetManager.Initialize(provider);

    assetManager.LoadTexture("player", "assets/player.png");

    TextureHandle first = assetManager.GetTextureHandle("player");
    TextureHandle second = assetManager.GetTextureHandle("player");

    REQUIRE(first.handle == second.handle);
}

TEST_CASE("Render.AssetManager.ClearAll_DestroysEveryLoadedTexture", "[AssetManager]")
{
    MockAssetProvider provider;
    AssetManager assetManager;
    assetManager.Initialize(provider);

    assetManager.LoadTexture("player", "assets/player.png");
    assetManager.LoadTexture("enemy", "assets/enemy.png");

    assetManager.ClearAll();

    REQUIRE(provider.destroyCallCount == 2);
}

TEST_CASE("Render.AssetManager.ClearAll_ThenGetTextureHandleReturnsInvalid", "[AssetManager]")
{
    MockAssetProvider provider;
    AssetManager assetManager;
    assetManager.Initialize(provider);

    assetManager.LoadTexture("player", "assets/player.png");
    assetManager.ClearAll();

    REQUIRE_FALSE(assetManager.GetTextureHandle("player").IsValid());
}

TEST_CASE("Render.AssetManager.ClearAll_AllowsReloadingSameId", "[AssetManager]")
{
    // After clearing, the cache is empty — loading the same id again
    // should reach the provider, not be silently skipped as "already cached".
    MockAssetProvider provider;
    AssetManager assetManager;
    assetManager.Initialize(provider);

    assetManager.LoadTexture("player", "assets/player.png");
    assetManager.ClearAll();
    assetManager.LoadTexture("player", "assets/player.png");

    REQUIRE(provider.loadCallCount == 2);
    REQUIRE(assetManager.GetTextureHandle("player").IsValid());
}