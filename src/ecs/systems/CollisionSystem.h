#pragma once

#include "ecs/core/System.h"

#include "ecs/components/TransformComponent.h"
#include "ecs/components/PhysicsComponent.h"

#include "godot_cpp/classes/physics_server3d.hpp"

using namespace godot;

class CollisionSystem : public System {
public:
    CollisionSystem(std::shared_ptr<Registry> _registry, std::shared_ptr<EntityCommandBuffer> _buffer)
        : System(_registry, _buffer)
        , query(_registry->query<TransformComponent, PhysicsComponent>())
    {}

    Registry::QueryResult<TransformComponent, PhysicsComponent> query;


    void update(const float& _dt) override {
        //if (!world_rid.is_valid())
        //    return;

        PhysicsServer3D* physics_server = PhysicsServer3D::get_singleton();

        for (auto [entity, transform_data, physics_data] : query) {
            //transform_data.transform = physics_server->body_get_state(physics_data.body_rid, PhysicsServer3D::BODY_STATE_TRANSFORM);
        }
    }
};
