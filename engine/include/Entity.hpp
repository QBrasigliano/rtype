#pragma once

#include "Component.hpp"
#include <unordered_map>
#include <memory>
#include <typeinfo>
#include <typeindex>

class Entity {
private:
    static uint32_t next_id_;
    uint32_t id_;
    std::unordered_map<std::type_index, std::shared_ptr<Component>> components_;

public:
    Entity();

    uint32_t GetId() const { return id_; }

    // Add or replace a component
    template <typename T, typename... Args>
    T& AddComponent(Args&&... args) {
        auto component = std::make_shared<T>(std::forward<Args>(args)...);
        components_[std::type_index(typeid(T))] = component;
        return *component;
    }

    // Get a component (throws if not found)
    template <typename T>
    T& GetComponent() {
        auto it = components_.find(std::type_index(typeid(T)));
        if (it == components_.end()) {
            throw std::runtime_error("Component not found");
        }
        return *std::static_pointer_cast<T>(it->second);
    }

    // Check if entity has component
    template <typename T>
    bool HasComponent() const {
        return components_.find(std::type_index(typeid(T))) != components_.end();
    }

    // Remove a component
    template <typename T>
    void RemoveComponent() {
        components_.erase(std::type_index(typeid(T)));
    }
};
