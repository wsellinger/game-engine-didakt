#include "SceneLoader.h"

#include "../ecs/EntityFactory.h"
#include "../core/Logger.h"

#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;

namespace
{
    std::vector<AssetEntry> ParseAssets(const json& assetsJson)
    {
        std::vector<AssetEntry> assets;
        if (assetsJson.is_array())
        {
            for (const auto& assetJson : assetsJson)
            {
                AssetEntry asset;
                asset.id = assetJson.value("id", "");
                asset.path = assetJson.value("path", "");
                assets.push_back(asset);
            }
        }
        return assets;
    }

    EntityDefinition ParseEntityDefinition(const json& entityJson)
    {
        EntityDefinition definition;

        //BoxCollider
        if (entityJson.contains("boxCollider"))
        {
            json componentJson = entityJson["boxCollider"];
            json offsetJson = componentJson.value("offset", json::array({ 0.0f, 0.0f }));

            BoxColliderComponent boxCollider;
            boxCollider.width = componentJson.value("width", 0);
            boxCollider.height = componentJson.value("height", 0);
            boxCollider.offset = { offsetJson[0].get<float>(), offsetJson[1].get<float>() };
            
            definition.boxCollider = boxCollider;
        }

        //Render
        if (entityJson.contains("render"))
        {
            json componentJson = entityJson["render"];

            RenderComponent render;
            render.textureId = componentJson.value("textureId", "");
            render.zIndex = componentJson.value("zIndex", 0);
            
            definition.render = render;
        }

        //RigidBody
        if (entityJson.contains("rigidBody"))
        {
            json componentJson = entityJson["rigidBody"];
            json velocityJson = componentJson.value("velocity", json::array({ 0.0f, 0.0f }));

            RigidBodyComponent rigidBody;
            rigidBody.velocity = { velocityJson[0].get<float>(), velocityJson[1].get<float>() };

            definition.rigidBody = rigidBody;
        }

        //SpriteSheet
        if (entityJson.contains("spriteSheet"))
        {
            json componentJson = entityJson["spriteSheet"];

            SpriteSheetComponent spriteSheet;
            spriteSheet.x = componentJson.value("x", 0);
            spriteSheet.y = componentJson.value("y", 0);
            spriteSheet.width = componentJson.value("width", 0);
            spriteSheet.height = componentJson.value("height", 0);

            definition.spriteSheet = spriteSheet;
        }

        //TileMap
        if (entityJson.contains("tileMap"))
        {
            json componentJson = entityJson["tileMap"];
            json tilesJson = componentJson.value("tiles", json::array());

            TileMapComponent tileMap;            
            tileMap.tileWidth = componentJson.value("tileWidth", 0);
            tileMap.tileHeight = componentJson.value("tileHeight", 0);
            tileMap.sheetColumns = componentJson.value("sheetColumns", 0);

            for (const auto& rowJson : tilesJson)
            {
                std::vector<int> tileRow;
                for (const auto& tileJson : rowJson)
                {
                    tileRow.push_back(tileJson.get<int>());
                }
                tileMap.tiles.push_back(tileRow);
            }
            
            definition.tileMap = tileMap;
        }

        //Transform
        if (entityJson.contains("transform"))
        {
            json componentJson = entityJson["transform"];
            json positionJson = componentJson.value("position", json::array({ 0.0f, 0.0f }));
            json scaleJson = componentJson.value("scale", json::array({ 1.0f, 1.0f }));

            TransformComponent transform;
            transform.position = { positionJson[0].get<float>(), positionJson[1].get<float>() };
            transform.rotation = componentJson.value("rotation", 0.0f);
            transform.scale = { scaleJson[0].get<float>(), scaleJson[1].get<float>() };

            definition.transform = transform;
        }

        return definition;
    }

    std::vector<EntityDefinition> ParseEntities(const json& entitiesJson)
    {
        std::vector<EntityDefinition> definitions;
        if (entitiesJson.is_array())
        {
            for (const auto& entityJson : entitiesJson)
            {
                EntityDefinition definition = ParseEntityDefinition(entityJson);
                definitions.push_back(definition);
            }
        }
        return definitions;
    }
}

std::optional<SceneLoadResult> SceneLoader::LoadFromFile(const std::string& path)
{
    //Load File
    std::ifstream file(path);
    if (!file.is_open())
    {
        Logger::Log(LogLevel::Error, "SceneLoader: Failed to open '%s'", path.c_str());
        return std::nullopt;
    }

    try
    {
        json sceneJson = json::parse(file);
        
        //Process Scene JSON
        Scene scene(sceneJson.value("name", "Unnamed"));
        std::vector<AssetEntry> assets = ParseAssets(sceneJson.value("assets", json::array()));
        std::vector<EntityDefinition> entityDefinitions = ParseEntities(sceneJson.value("entities", json::array()));

        //Populate Scene Registry
        for (const auto& definition : entityDefinitions)
            EntityFactory::CreateFromDefinition(scene.GetRegistry(), definition);

        return SceneLoadResult{ std::move(scene), std::move(assets) };
    }
    catch (const json::exception& e)
    {
        Logger::Log(LogLevel::Error, "SceneLoader: Failed to parse '%s': %s", path.c_str(), e.what());
        return std::nullopt;
    }
}
