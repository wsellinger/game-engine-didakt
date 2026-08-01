#include "RenderSystem.h"

#include "../../core/Assert.h"
#include "../../core/Logger.h"
#include "../../providers/interfaces/IRenderProvider.h"
#include "../../render/AssetManager.h"
#include "../../render/Camera.h"
#include "../../render/RenderMath.h"
#include "../components/RenderComponent.h"
#include "../components/SpriteSheetComponent.h"
#include "../components/TileMapComponent.h"
#include "../components/TransformComponent.h"

#include <algorithm>
#include <vector>

#include <entt/entity/fwd.hpp>
#include <entt/entity/registry.hpp>

#include <glm/ext/vector_float2.hpp>

void RenderSystem::Render(entt::registry& registry, IRenderProvider& renderProvider, AssetManager& assetManager, const Camera& camera)
{
    //Get Commands
    std::vector<DrawCommand> drawCommands;
    AppendCommands<SpriteSheetComponent, TransformComponent>(DrawType::Sprite, registry, drawCommands);
    AppendCommands<TileMapComponent>(DrawType::TileMap, registry, drawCommands);

    //Sort
    std::sort(drawCommands.begin(), drawCommands.end(), 
        [](const DrawCommand& a, const DrawCommand& b) { return a.zIndex < b.zIndex; });

    //Render
    RenderParameters renderParameters{ registry, renderProvider, assetManager, camera };
    for (auto drawCommand : drawCommands)
    {
        switch (drawCommand.type)
        {
        case DrawType::Sprite:
            RenderSprite(drawCommand.entity, renderParameters);
            break;
        case DrawType::TileMap:
            RenderTileMap(drawCommand.entity, renderParameters);
            break;
        default:
            Logger::Log(LogLevel::Error, "RenderSystem: unknown DrawType %d", static_cast<int>(drawCommand.type));
            break;
        }
    }
}

template<typename... Components>
void RenderSystem::AppendCommands(DrawType type, entt::registry& registry, std::vector<DrawCommand>& drawCommands)
{
    auto view = registry.view<RenderComponent, Components...>();
    for (auto entity : view)
        drawCommands.push_back({ entity, type, view.get<RenderComponent>(entity).zIndex });
}

void RenderSystem::RenderSprite(entt::entity entity, const RenderParameters& renderParameters)
{
    auto& [registry, renderProvider, assetManager, camera] = renderParameters;

    //Get Components
    auto view = registry.view<RenderComponent, SpriteSheetComponent, TransformComponent>();
    const auto& render = view.get<RenderComponent>(entity);
    const auto& sprite = view.get<SpriteSheetComponent>(entity);
    const auto& transform = view.get<TransformComponent>(entity);

    //Texture
    TextureHandle handle = assetManager.GetTextureHandle(render.textureId);
    DIDAKT_ASSERT(handle.IsValid());

    //Source
    RenderRect source{ sprite.x, sprite.y, sprite.width, sprite.height };

    //Destination
    glm::vec2 worldSize{ sprite.width * transform.scale.x, sprite.height * transform.scale.y };
    RenderRect destination = RenderMath::GetSpriteDestinationRect(transform.position, worldSize, camera);

    //Render
    renderProvider.DrawTexture(handle, source, destination, transform.rotation);
}

using TileRow = std::vector<int>;
using TileGrid = std::vector<TileRow>;

void RenderSystem::RenderTileMap(entt::entity entity, const RenderParameters& renderParameters)
{
    auto& [registry, renderProvider, assetManager, camera] = renderParameters;

    //Get Components
    auto view = registry.view<RenderComponent, TileMapComponent>();
    const auto& render = view.get<RenderComponent>(entity);
    const auto& tilemap = view.get<TileMapComponent>(entity);

    //Texture
    TextureHandle handle = assetManager.GetTextureHandle(render.textureId);
    DIDAKT_ASSERT(handle.IsValid());

    //Grid
    const TileGrid& tiles = tilemap.tiles;
    size_t numRows = tiles.size();

    //Tiles
    for (size_t iRow = 0; iRow < numRows; iRow++)
    {
        const TileRow& row = tiles[iRow];
        size_t numCol = row.size();

        for (size_t iCol = 0; iCol < numCol; iCol++)
        {
            int id = row[iCol];
            RenderRect source = RenderMath::GetTileSourceRect(id, tilemap.tileWidth, tilemap.tileHeight, tilemap.sheetColumns);
            RenderRect destination = RenderMath::GetTileDestinationRect(iRow, iCol, tilemap.tileWidth, tilemap.tileHeight, camera);

            renderProvider.DrawTexture(handle, source, destination, 0.0);
        }
    }
}