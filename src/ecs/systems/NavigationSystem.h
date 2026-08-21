#pragma once

#include "ecs/core/System.h"

#include "../components/VelocityComponent.h"
#include "../components/TransformComponent.h"
#include "../components/NavigationComponent.h"

#include "../../navigation/flowfield.h"

using namespace godot;

class NavigationSystem : public System {
public:
    NavigationSystem(std::shared_ptr<Registry> _registry, std::shared_ptr<EntityCommandBuffer> _buffer)
        : System(_registry, _buffer)
        , query(_registry->query<TransformComponent, VelocityComponent, NavigationComponent>())
    {}

    Registry::QueryResult<TransformComponent, VelocityComponent, NavigationComponent> query;

    void update(const float& _dt) override {
        for (auto [entity, transform_data, velocity_data, navigation_data] : query) {
            const int32_t region_id = flowfield.GetClosestNode(transform_data.transform.origin);
            if (region_id != navigation_data.region_id)
                navigation_data.region_id = region_id;

            Vector3 safe_direction = Avoidance(transform_data.transform.origin, navigation_data.move_speed, entity, region_id);

            navigation_data.direction = safe_direction;

            //temp, testing purposes
            velocity_data.velocity = navigation_data.direction * navigation_data.move_speed;
            velocity_data.velocity.y -= 9.8;
        }
    }


    auto Avoidance(const Vector3& _position, float _move_speed, Entity _entity, int32_t _region_id) -> Vector3 {
        const FlowField::NavigationNode& node = flowfield.node_list[_region_id];
        Vector3 direction = node.ideal_direction;

        if (_region_id == flowfield.target_id) {
            if (_position.distance_to(flowfield.target_position) > 0.1)
                return _position.direction_to(flowfield.target_position);
            else
                return Vector3(0, 0, 0);
        }

        const std::vector<Entity>& relevant_entities = region_entities[_region_id];

        for (Entity other_entity : relevant_entities) {
            if (other_entity == _entity)
                continue;

            TransformComponent* other_transform = registry->tryGetComponent<TransformComponent>(other_entity);
            if (!other_transform)
                continue;

            const Vector3& other_pos = other_transform->transform.origin;

            const Vector3 predicted_pos = _position + node.ideal_direction * _move_speed;
            const auto dist = predicted_pos.distance_to(other_pos);

            if (dist < avoidance_radius) {
                const Vector3 to_other = (_position - other_pos).normalized();
                const Vector3 right = to_other.cross(Vector3(0, 1, 0)).normalized();

                const auto right_dot = node.ideal_direction.dot(right);

                const Vector3 avoidance_dir = right_dot > 0 ? right : -right;
                const float avoidance_str = (avoidance_radius - dist) / avoidance_radius;

                direction += avoidance_dir * avoidance_str;
                direction.normalize();
            }
        }

        return direction;
    }

    void RegisterEntity(Entity _entity) {
        TransformComponent* transform = registry->tryGetComponent<TransformComponent>(_entity);
        NavigationComponent* navigation = registry->tryGetComponent<NavigationComponent>(_entity);
        if (!transform || !navigation)
            return;

        int32_t region_id = flowfield.GetClosestNode(transform->transform.origin);        
        //region_entities[region_id].push_back(_entity);
        
        navigation->region_id = region_id;
    }

    void UnregisterEntity(const Entity _entity) {
        NavigationComponent* navigation = registry->tryGetComponent<NavigationComponent>(_entity);
        if (!navigation)
            return;

        int32_t& region_id = navigation->region_id;
        auto it = std::find(region_entities[region_id].begin(), region_entities[region_id].end(), _entity);

        if (it != region_entities[region_id].end())
            region_entities[region_id].erase(it);
    }

    void GenerateNodes(Ref<NavigationMesh> _nav_mesh) {
        flowfield.GenerateNodes(_nav_mesh);
        region_entities.resize(_nav_mesh->get_polygon_count());
    }


     
    FlowField flowfield;
    
    std::vector<std::vector<Entity>> region_entities;

    float avoidance_radius = 2.0f;
};
