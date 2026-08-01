cmake_minimum_required(VERSION 3.20)
project(Didakt VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#=== Fetch

include(FetchContent)

#SDL2
FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        release-2.30.3
)
FetchContent_MakeAvailable(SDL2)

#SDL2_image
FetchContent_Declare(
    SDL2_image
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
    GIT_TAG        release-2.8.2
)
set(SDL2IMAGE_INSTALL OFF)
FetchContent_MakeAvailable(SDL2_image)

#nlohmann_json
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

#EnTT
FetchContent_Declare(
    entt
    GIT_REPOSITORY https://github.com/skypjack/entt.git
    GIT_TAG        v3.13.2
)
FetchContent_MakeAvailable(entt)

#GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
)
set(GLM_BUILD_LIBRARY OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glm)

#=== Providers

include(cmake/Providers.cmake)

#==========
#=== Engine

#=== Executable

add_executable(Didakt src/main.cpp
    #Core
    "src/core/Assert.h"
    "src/core/Engine.h"      "src/core/Engine.cpp"
    "src/core/Config.h"      "src/core/Config.cpp"
    "src/core/LogLevel.h"
    "src/core/Logger.h"      "src/core/Logger.cpp"

    #Providers
    ${DIDAKT_PROVIDER_SOURCES}

    #Entity Component System
    "src/ecs/EntityDefinition.h"
    "src/ecs/EntityFactory.h"   "src/ecs/EntityFactory.cpp"

    #-Components
    "src/ecs/components/BoxColliderComponent.h"
    "src/ecs/components/RenderComponent.h"
    "src/ecs/components/RigidBodyComponent.h"
    "src/ecs/components/SpriteSheetComponent.h"
    "src/ecs/components/TileMapComponent.h"
    "src/ecs/components/TransformComponent.h"

    #-Systems
    "src/ecs/systems/MovementSystem.h" "src/ecs/systems/MovementSystem.cpp"
    "src/ecs/systems/RenderSystem.h"   "src/ecs/systems/RenderSystem.cpp"

    #Events
    "src/events/EventBus.h"

    #Input
    "src/input/InputManager.h" "src/input/InputManager.cpp"
    "src/input/Key.h"
    "src/input/MouseButton.h"

    #Render
    "src/render/AssetManager.h" "src/render/AssetManager.cpp"
    "src/render/Camera.h"
    "src/render/RenderColor.h"
    "src/render/RenderRect.h"
    "src/render/RenderMath.h"   "src/render/RenderMath.cpp"
    "src/render/TextureHandle.h"

    #Scenes
    "src/scenes/Scene.h"
    "src/scenes/SceneLoader.h"  "src/scenes/SceneLoader.cpp"
    "src/scenes/SceneManager.h" "src/scenes/SceneManager.cpp"
)

#=== Libraries

didakt_configure_provider_target(Didakt)
didakt_link_sdl2main(Didakt)

target_link_libraries(Didakt PRIVATE
    nlohmann_json::nlohmann_json
    EnTT::EnTT
    glm::glm
)

#=== Post Build

add_custom_target(CopyConfig ALL
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/config/config.json
        $<TARGET_FILE_DIR:Didakt>/config/config.json
)

add_dependencies(Didakt CopyConfig)

#=========
#=== Tests

#=== Fetch

#Catch2
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.7.1
)
FetchContent_MakeAvailable(Catch2)
enable_testing()

include(CTest)
include(Catch)

#=== Executable

add_executable(DidaktTests
    #Tests
    "tests/core/ConfigTests.cpp"
    "tests/core/EngineTimingTests.cpp"
    "tests/core/LoggerTests.cpp"

    "tests/ecs/EntityFactoryTests.cpp"
    "tests/ecs/MovementSystemTests.cpp"
    "tests/ecs/RenderSystemTests.cpp"

    "tests/events/EventBusTests.cpp"

    "tests/input/InputManagerTests.cpp"

    "tests/mocks/MockAssetProvider.h" 
    "tests/mocks/TempJsonFile.h"

    "tests/render/AssetManagerTests.cpp"
    "tests/render/CameraTests.cpp"
    "tests/render/RenderMathTests.cpp"
    
    "tests/scenes/SceneTests.cpp"
    "tests/scenes/SceneLoaderTests.cpp"
    "tests/scenes/SceneManagerTests.cpp"

    #Providers
    ${DIDAKT_PROVIDER_SOURCES}

    #Source
    "src/core/Config.cpp"
    "src/core/Engine.cpp"
    "src/core/Logger.cpp"

    "src/ecs/EntityFactory.cpp" 
    "src/ecs/systems/MovementSystem.cpp"
    "src/ecs/systems/RenderSystem.cpp"

    "src/input/InputManager.cpp"

    "src/render/AssetManager.cpp"
    "src/render/RenderMath.cpp"
    
    "src/scenes/SceneLoader.cpp"
    "src/scenes/SceneManager.cpp" 
)

#=== Include Directories

target_include_directories(DidaktTests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

#=== Libraries

didakt_configure_provider_target(DidaktTests)

target_link_libraries(DidaktTests PRIVATE
    Catch2::Catch2WithMain
    nlohmann_json::nlohmann_json
    EnTT::EnTT
    glm::glm
)

#=== Post Build

add_custom_command(TARGET DidaktTests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Catch2::Catch2>
        $<TARGET_FILE_DIR:DidaktTests>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Catch2::Catch2WithMain>
        $<TARGET_FILE_DIR:DidaktTests>
)

catch_discover_tests(DidaktTests)