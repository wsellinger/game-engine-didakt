#pragma once

#include <entt/entt.hpp>
#include <string>

class Scene
{
public:
    explicit Scene(const std::string& name) : _name(name) { }

    entt::registry& GetRegistry() { return _registry; }
    const entt::registry& GetRegistry() const { return _registry; }
    const std::string& GetName() const { return _name; }

private:
    std::string _name;
    entt::registry _registry;
};