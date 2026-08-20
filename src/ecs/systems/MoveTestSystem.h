#pragma once

#include "ecs/core/System.h"

#include "ecs/components/TransformComponent.h"

using namespace godot;

class MoveTestSystem : public System {
public:
    MoveTestSystem(std::shared_ptr<Registry> _registry, std::shared_ptr<EntityCommandBuffer> _buffer)
        : System(_registry, _buffer)
        , query(_registry->query<TransformComponent>())
    {}

    Registry::QueryResult<TransformComponent> query;


    float time = 0.f;
    void update(const float& _dt) override {
        time += _dt;

        for (auto [entity, transform_data] : query) 
            transform_data.transform.origin.y += sin(time + int(entity) * 0.01f);
    }
};
