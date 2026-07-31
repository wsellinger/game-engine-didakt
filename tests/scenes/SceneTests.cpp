#include "scenes/Scene.h"

#include <catch2/catch_test_macros.hpp>

//=== Helpers ===

// Exists purely to prove the const-qualified GetRegistry() overload
// resolves correctly when only a const reference is available.
static std::size_t CountEntitiesConst(const Scene& scene)
{
    const entt::registry& registry = scene.GetRegistry();
    return registry.storage<entt::entity>()->size();
}

//=== Tests ===

TEST_CASE("Scene.Construction_StoresName", "[Scene]")
{
    Scene scene("TestScene");
    REQUIRE(scene.GetName() == "TestScene");
}

TEST_CASE("Scene.Registry_StartsEmpty", "[Scene]")
{
    Scene scene("TestScene");
    REQUIRE(scene.GetRegistry().storage<entt::entity>().size() == 0);
}

TEST_CASE("Scene.Registry_MutationPersists", "[Scene]")
{
    Scene scene("TestScene");

    entt::entity entity = scene.GetRegistry().create();

    // Second call to GetRegistry() should see the same underlying registry,
    // not a copy — this is the thing a reference-vs-value typo would break.
    REQUIRE(scene.GetRegistry().valid(entity));
    REQUIRE(scene.GetRegistry().storage<entt::entity>().size() == 1);
}

TEST_CASE("Scene.Registry_ConstOverloadResolves", "[Scene]")
{
    Scene scene("TestScene");
    scene.GetRegistry().create();
    scene.GetRegistry().create();

    REQUIRE(CountEntitiesConst(scene) == 2);
}

TEST_CASE("Scene.Destruction_DoesNotCrash", "[Scene]")
{
    {
        Scene scene("Ephemeral");
        scene.GetRegistry().create();
        scene.GetRegistry().create();
        scene.GetRegistry().create();
    }
    // Nothing to assert — this exists to exercise destruction under
    // whatever sanitizer/valgrind runs against the test suite.
    SUCCEED();
}