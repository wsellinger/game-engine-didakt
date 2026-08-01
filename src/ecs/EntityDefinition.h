#pragma once

#include "components/TransformComponent.h"
#include "components/RenderComponent.h"
#include "components/RigidBodyComponent.h"
#include "components/BoxColliderComponent.h"
#include "components/SpriteSheetComponent.h"

#include <optional>

struct EntityDefinition
{
    std::optional<TransformComponent> transform;
    std::optional<RenderComponent> render;
    std::optional<RigidBodyComponent> rigidBody;
    std::optional<BoxColliderComponent> boxCollider;
    std::optional<SpriteSheetComponent> spriteSheet;
};