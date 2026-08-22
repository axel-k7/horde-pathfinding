#pragma once

#include "ecs/core/System.h"

#include "ecs/components/TransformComponent.h"
#include "ecs/components/VelocityComponent.h"
#include "ecs/components/PhysicsComponent.h"

#include "godot_cpp/classes/physics_server3d.hpp"
#include "godot_cpp/classes/physics_test_motion_parameters3d.hpp"
#include "godot_cpp/classes/physics_test_motion_result3d.hpp"

using namespace godot;

class PhysicsSystem : public System {
public:
    PhysicsSystem(std::shared_ptr<Registry> _registry, std::shared_ptr<EntityCommandBuffer> _buffer)
        : System(_registry, _buffer)
        , query(_registry->query<TransformComponent, VelocityComponent, PhysicsComponent>())
    {}

    Registry::QueryResult<TransformComponent, VelocityComponent, PhysicsComponent> query;

    const int max_slide = 1;

    void update(const float& _dt) override {
        auto physics_server = PhysicsServer3D::get_singleton();

        Ref<PhysicsTestMotionParameters3D> parameters;
        parameters.instantiate();
        parameters->set_margin(0.04f);

        Ref<PhysicsTestMotionResult3D> results;
        results.instantiate();

        for (auto [entity, transform_data, velocity_data, physics_data]: query) {
            Vector3 motion = velocity_data.velocity * _dt;
            if (motion.is_zero_approx())
                continue;
            
            Transform3D curr_transform = transform_data.transform;
            bool grounded = false;

            for (int i = 0; i < max_slide; i++) {
                if (motion.is_zero_approx())
                    break;

                parameters->set_from(curr_transform);
                parameters->set_motion(motion);
                

                bool collision = physics_server->body_test_motion(physics_data.body_rid, parameters, results);

                if (!collision) {
                    curr_transform.origin += motion;
                    break;
                }

                Vector3 to_collision = results->get_travel();
                curr_transform.origin += to_collision;

                Vector3 remainder = results->get_remainder();
                Vector3 normal = results->get_collision_normal();

                if (normal.y > 0.7) //~45deg
                    grounded = true;

                motion = remainder - normal * remainder.dot(normal); 
            }

            if (grounded)
                velocity_data.velocity.y = 0;

            transform_data.transform = curr_transform;
            physics_server->body_set_state(physics_data.body_rid, PhysicsServer3D::BODY_STATE_TRANSFORM, curr_transform);
        }
    }
};
