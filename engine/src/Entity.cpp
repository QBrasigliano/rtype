#include "../include/Entity.hpp"

uint32_t Entity::next_id_ = 0;

Entity::Entity() : id_(next_id_++) {}
