#include "ecs/EntityFactory.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("EntityFactory.CreateFromDefinition_AllComponentsPresent", "[EntityFactory]")
{
    entt::registry registry;

    EntityDefinition definition;
    definition.transform = TransformComponent{ glm::vec2{10.f, 20.f}, 45.f, glm::vec2{2.f, 2.f} };
    definition.render = RenderComponent{ "player", 3 };
    definition.rigidBody = RigidBodyComponent{ glm::vec2{1.f, -1.f} };
    definition.boxCollider = BoxColliderComponent{ 16, 16, glm::vec2{0.f, 0.f} };
    definition.spriteSheet = SpriteSheetComponent{ 0, 0, 32, 32 };

    entt::entity entity = EntityFactory::CreateFromDefinition(registry, definition);

    REQUIRE(registry.valid(entity));

    REQUIRE(registry.all_of<TransformComponent, RenderComponent, RigidBodyComponent,
        BoxColliderComponent, SpriteSheetComponent>(entity));

    const auto& transform = registry.get<TransformComponent>(entity);
    REQUIRE(transform.position.x == 10.f);
    REQUIRE(transform.position.y == 20.f);
    REQUIRE(transform.rotation == 45.f);

    const auto& render = registry.get<RenderComponent>(entity);
    REQUIRE(render.textureId == "player");
    REQUIRE(render.zIndex == 3);

    const auto& rigidBody = registry.get<RigidBodyComponent>(entity);
    REQUIRE(rigidBody.velocity.x == 1.f);
    REQUIRE(rigidBody.velocity.y == -1.f);
}

TEST_CASE("EntityFactory.CreateFromDefinition_NoComponentsPresent", "[EntityFactory]")
{
    entt::registry registry;
    EntityDefinition definition; // all fields left empty

    entt::entity entity = EntityFactory::CreateFromDefinition(registry, definition);

    REQUIRE(registry.valid(entity));
    REQUIRE_FALSE(registry.any_of<TransformComponent, RenderComponent, RigidBodyComponent,
        BoxColliderComponent, SpriteSheetComponent>(entity));
}

TEST_CASE("EntityFactory.CreateFromDefinition_PartialSubset", "[EntityFactory]")
{
    entt::registry registry;

    EntityDefinition definition;
    definition.transform = TransformComponent{ glm::vec2{5.f, 5.f}, 0.f, glm::vec2{1.f, 1.f} };
    definition.render = RenderComponent{ "crate", 1 };
    // rigidBody, boxCollider, spriteSheet left empty

    entt::entity entity = EntityFactory::CreateFromDefinition(registry, definition);

    REQUIRE(registry.all_of<TransformComponent, RenderComponent>(entity));
    REQUIRE_FALSE(registry.any_of<RigidBodyComponent, BoxColliderComponent, SpriteSheetComponent>(entity));
}