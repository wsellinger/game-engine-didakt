#pragma once

#include "EntityDefinition.h"

#include <entt/entt.hpp>

class EntityFactory
{
public:
    EntityFactory() = delete;

    static entt::entity CreateFromDefinition(entt::registry& registry, const EntityDefinition& definition);

private:
    template<typename Component>
    static void EmplaceOptional(entt::registry& registry, entt::entity entity, const std::optional<Component>& component)
    {
        if (component)
            registry.emplace<Component>(entity, *component);
    }
};