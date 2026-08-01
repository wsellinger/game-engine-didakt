#pragma once

#include "components/BoxColliderComponent.h"
#include "components/RenderComponent.h"
#include "components/RigidBodyComponent.h"
#include "components/SpriteSheetComponent.h"
#include "components/TileMapComponent.h"
#include "components/TransformComponent.h"

#include <optional>

struct EntityDefinition
{
    std::optional<BoxColliderComponent> boxCollider;
    std::optional<RenderComponent> render;
    std::optional<RigidBodyComponent> rigidBody;
    std::optional<SpriteSheetComponent> spriteSheet;
    std::optional<TileMapComponent> tileMap;
    std::optional<TransformComponent> transform;
};