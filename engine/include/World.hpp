#pragma once

#include "Entity.hpp"
#include <vector>
#include <memory>
#include <algorithm>

class World {
private:
    std::vector<std::shared_ptr<Entity>> entities_;

public:
    // Create a new entity
    std::shared_ptr<Entity> CreateEntity() {
        auto entity = std::make_shared<Entity>();
        entities_.push_back(entity);
        return entity;
    }

    // Get all entities
    const std::vector<std::shared_ptr<Entity>>& GetEntities() const {
        return entities_;
    }

    // Remove an entity by id
    void RemoveEntity(uint32_t id) {
        entities_.erase(
            std::remove_if(entities_.begin(), entities_.end(),
                [id](const std::shared_ptr<Entity>& e) { return e->GetId() == id; }),
            entities_.end()
        );
    }

    // Remove an entity by shared_ptr
    void RemoveEntity(const std::shared_ptr<Entity>& entity) {
        entities_.erase(
            std::remove(entities_.begin(), entities_.end(), entity),
            entities_.end()
        );
    }

    // Clear all entities
    void Clear() {
        entities_.clear();
    }

    // Get entity by id
    std::shared_ptr<Entity> GetEntity(uint32_t id) {
        for (auto& entity : entities_) {
            if (entity->GetId() == id) {
                return entity;
            }
        }
        return nullptr;
    }
};
