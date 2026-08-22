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

    struct NavigationResult {
        Vector3 direction;
        float speed_scale;
    };

    ///gotta make sure to work around this if multithreading
    std::unordered_map<Entity, Vector3, EntityHash> position_cache;

    void update(const float& _dt) override {
        position_cache.clear();
        for (auto [entity, transform_data, velocity_data, navigation_data] : query) {
            position_cache.emplace(entity, transform_data.transform.origin);
            
            const int32_t region_id = flowfield.CheckNodeChanged(transform_data.transform.origin, navigation_data.region_id);
    
            if (region_id != navigation_data.region_id) {
                auto& curr_array = region_entities[navigation_data.region_id];
                auto it = std::find(curr_array.begin(), curr_array.end(), entity);
            
                if (it != curr_array.end()) {
                    *it = curr_array.back();
                    curr_array.pop_back();
                }

                region_entities[region_id].push_back(entity);
                navigation_data.region_id = region_id;
            }
        }
                
        for (auto [entity, transform_data, velocity_data, navigation_data] : query) {
            NavigationResult result = GetDirection(transform_data.transform.origin, navigation_data.move_speed, entity, navigation_data.region_id);
            navigation_data.direction = result.direction;

            Vector3 desired_dir = navigation_data.direction * navigation_data.move_speed * result.speed_scale;
            
            velocity_data.velocity.x = desired_dir.x;
            velocity_data.velocity.y -= 9.8 * _dt;
            velocity_data.velocity.z = desired_dir.z;
        }
    }

    auto GetDirection(const Vector3& _position, float _move_speed, Entity _entity, int32_t _region_id) -> NavigationResult {
        const FlowField::NavigationNode& node = flowfield.node_list[_region_id];
        
        NavigationResult result = {node.ideal_direction, 1.f};

        if (_region_id == flowfield.target_id) {
            if (_position.distance_to(flowfield.target_position) > 0.1) {
                result.direction =  _position.direction_to(flowfield.target_position);
                return result;
            }
            result.direction = Vector3(0,0,0);
            return result;
        }

        Vector3 direction = node.ideal_direction;
        if (avoidance_on)
            result = Avoidance(_position, _move_speed, _entity, _region_id);

        result.direction.y = 0;
        result.direction.normalize();
        return result;
    }

    auto Avoidance(const Vector3& _position, float _move_speed, Entity _entity, int32_t _region_id) -> NavigationResult {
        const FlowField::NavigationNode& node = flowfield.node_list[_region_id];
        
        Vector3 direction = node.ideal_direction;
        const Vector3 predicted_pos = _position + node.ideal_direction * _move_speed;

        float surrounded_factor = 0.f;

        for (Entity other_entity : region_entities[_region_id]) {
            if (other_entity == _entity)
                continue;

            auto it = position_cache.find(other_entity);
            if (it == position_cache.end())
                continue;

            const Vector3& other_pos = it->second;

            const auto dist = predicted_pos.distance_to(other_pos);
            if (dist > avoidance_radius)
                continue;

            const Vector3 to_other = _position - other_pos;

            const Vector3 right = to_other.cross(Vector3(0, 1, 0)).normalized();
            const auto right_dot = node.ideal_direction.dot(right);
            
            const Vector3 avoidance_dir = right_dot > 0 ? right : -right;
            const float avoidance_str = (avoidance_radius - dist) / avoidance_radius;
            
            direction += avoidance_dir * avoidance_str;

            //slow down if another entity is blocking path to avoid collision
            float block_factor = node.ideal_direction.dot(to_other);
            if (block_factor > 0.f)
                surrounded_factor += block_factor*avoidance_str;
        }

        float speed_scale = 1.f/(1.f+surrounded_factor);

        return {direction, speed_scale};
    }

    void RegisterEntity(Entity _entity) {
        TransformComponent* transform = registry->tryGetComponent<TransformComponent>(_entity);
        NavigationComponent* navigation = registry->tryGetComponent<NavigationComponent>(_entity);
        if (!transform || !navigation)
            return;

        int32_t region_id = flowfield.GetClosestNode(transform->transform.origin); 

        region_entities[region_id].push_back(_entity);
        
        navigation->region_id = region_id;
    }

    void UnregisterEntity(const Entity _entity) {
        NavigationComponent* navigation = registry->tryGetComponent<NavigationComponent>(_entity);
        if (!navigation)
            return;

        int32_t& region_id = navigation->region_id;
        auto& array = region_entities[region_id];

        auto it = std::find(array.begin(),array.end(), _entity);
        if (it != array.end()) {
            *it = array.back();
            array.pop_back();
        }
    }

    void GenerateNodes(Ref<NavigationMesh> _nav_mesh) {
        flowfield.GenerateNodes(_nav_mesh);
        region_entities.resize(_nav_mesh->get_polygon_count());
    }


    FlowField flowfield;
    
    std::vector<std::vector<Entity>> region_entities;

    float avoidance_radius = 2.0f;

    bool avoidance_on = true;
};
