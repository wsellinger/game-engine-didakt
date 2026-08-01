#include "EntityFactory.h"

entt::entity EntityFactory::CreateFromDefinition(entt::registry& registry, const EntityDefinition& definition)
{
    entt::entity entity = registry.create();

    EmplaceOptional(registry, entity, definition.transform);
    EmplaceOptional(registry, entity, definition.render);
    EmplaceOptional(registry, entity, definition.rigidBody);
    EmplaceOptional(registry, entity, definition.boxCollider);
    EmplaceOptional(registry, entity, definition.spriteSheet);
    EmplaceOptional(registry, entity, definition.tileMap);

    return entity;
}

