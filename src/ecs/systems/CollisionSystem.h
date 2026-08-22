#pragma once

#include "ecs/core/System.h"

#include "ecs/components/TransformComponent.h"
#include "ecs/components/PhysicsComponent.h"

#include "godot_cpp/classes/physics_server3d.hpp"

using namespace godot;

class CollisionSystem : public System {
public:
    CollisionSystem(std::shared_ptr<Registry> _registry, std::shared_ptr<EntityCommandBuffer> _buffer, NavigationSystem* _nav_sys)
        : System(_registry, _buffer)
        , query(_registry->query<TransformComponent, VelocityComponent, PhysicsComponent, NavigationComponent>())
        , navigation_system(_nav_sys)
    {}

    Registry::QueryResult<TransformComponent, VelocityComponent, PhysicsComponent, NavigationComponent> query;

    NavigationSystem* navigation_system;
    float collision_radius = 1.f;

    //way cheaper to do entity to entity collison within in a horde this way with barely any noticable change

    void update(const float& _dt) override {
        const auto& region_entities = navigation_system->region_entities;
        const auto& position_cache = navigation_system->position_cache;

        for (auto [entity, transform_data, velocity_data, physics_data, navigation_data] : query) {
            Vector3 separation_force = Vector3(0, 0, 0);

            for (Entity other_entity : region_entities[navigation_data.region_id]) {
                if (other_entity == entity) continue;

                auto it = position_cache.find(other_entity);
                if (it == position_cache.end()) continue;

                Vector3 to_entity = transform_data.transform.origin - it->second;
                to_entity.y = 0;

                float dist_sqr = to_entity.length_squared();
                bool overlapping = dist_sqr < 0.0001f;

                if (dist_sqr < collision_radius * collision_radius && !overlapping) {
                    float dist = Math::sqrt(dist_sqr);
                    float overlap = collision_radius - dist;

                    separation_force += (to_entity / dist) * (overlap / _dt) * 0.5f; 
                } else if (overlapping) {
                    //don't have a clear separation vector since entities are inside eachother
                    //"random" deterministic direction is calculated
                    separation_force += Vector3(cos(entity.id), 0, sin(entity.id)) * (collision_radius / _dt) * 0.5f;
                }
            }

            velocity_data.velocity += separation_force;
        }
    }
};
