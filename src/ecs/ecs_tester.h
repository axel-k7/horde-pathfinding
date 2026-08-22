#pragma once

#include "godot_cpp/classes/engine.hpp"
#include "godot_cpp/classes/node3d.hpp"
#include "godot_cpp/classes/world3d.hpp"
#include "godot_cpp/classes/material.hpp"
#include "godot_cpp/classes/navigation_mesh.hpp"
#include "godot_cpp/classes/concave_polygon_shape3d.hpp"
#include "godot_cpp/classes/time.hpp"

#include "systems/RenderingSystem.h"
#include "systems/PhysicsSystem.h"
#include "systems/NavigationSystem.h"
#include "systems/CollisionSystem.h"

#include "ecs/core/EntityCommandBuffer.h"

using namespace godot;

class ECSTester : public Node3D {
    GDCLASS(ECSTester, Node3D)

private:
    std::shared_ptr<Registry> registry;
    std::shared_ptr<EntityCommandBuffer> buffer;

    std::unique_ptr<RenderingSystem> render_sys;
    std::unique_ptr<PhysicsSystem> physics_sys;
    std::unique_ptr<CollisionSystem> collision_sys;
    std::unique_ptr<NavigationSystem> nav_sys;

    Ref<NavigationMesh> nav_mesh;

    Ref<Mesh> test_mesh;
    Ref<Material> test_material;
    int entity_count = 100;

    const uint32_t enviroment_layer = 1;
    const uint32_t horde_layer = 2; //they shouldn't need collision checks with eachother if avoidance works properly

public:
    void _ready() override { 
        if (Engine::get_singleton()->is_editor_hint()) 
            return;

        registry = std::make_shared<Registry>();
        buffer = std::make_shared<EntityCommandBuffer>(registry);

        physics_sys = std::make_unique<PhysicsSystem>(registry, buffer);

        RID world_context = get_world_3d()->get_scenario();
        render_sys = std::make_unique<RenderingSystem>(
            registry,
            buffer,
            world_context
        );

        nav_sys = std::make_unique<NavigationSystem>(registry, buffer);
        nav_sys->GenerateNodes(nav_mesh);

        collision_sys = std::make_unique<CollisionSystem>(registry, buffer, nav_sys.get());

        PhysicsServer3D* physics_server = PhysicsServer3D::get_singleton();
        auto space_rid = get_world_3d()->get_space();
        auto shape_rid = physics_server->capsule_shape_create();
        godot::Dictionary shape_data;
        shape_data[StringName("radius")] = 0.5f;
        shape_data[StringName("height")] = 2.0f;

        physics_server->shape_set_data(shape_rid, shape_data);

        for (int i = 0; i < entity_count; i++) {
            Entity entity = registry->allocateEntity();

            registry->addComponents<TransformComponent, VelocityComponent, MeshComponent, PhysicsComponent, NavigationComponent>(
                entity, {}, {}, {}, {}, {float(UtilityFunctions::randf_range(5, 15))});
            
            auto mesh = registry->tryGetComponent<MeshComponent>(entity);
            mesh->mesh_rid = test_mesh->get_rid();
            mesh->material_rid = test_material->get_rid();

            auto transform = registry->tryGetComponent<TransformComponent>(entity);
            transform->transform.origin = Vector3(
                UtilityFunctions::randf_range(-45, 45),
                2,
                UtilityFunctions::randf_range(-45, 45)
            );

            auto physics = registry->tryGetComponent<PhysicsComponent>(entity);
            auto body_rid = physics_server->body_create();
            physics_server->body_set_space(body_rid, space_rid);
            physics_server->body_add_shape(body_rid, shape_rid, Transform3D());
            physics_server->body_set_mode(body_rid, PhysicsServer3D::BODY_MODE_KINEMATIC);
            physics_server->body_set_state(body_rid, PhysicsServer3D::BODY_STATE_TRANSFORM, transform->transform);
            physics_server->body_set_collision_layer(body_rid, horde_layer);
            physics_server->body_set_collision_mask(body_rid, enviroment_layer); //only collide with enviroment
            physics->body_rid = body_rid;
            physics->shape_rid = shape_rid;
            physics->collision_layer = 1;

            nav_sys->RegisterEntity(entity);
        }
        
        nav_sys->flowfield.GenerateFlowField(Vector3(45.f,0,45.f));
    }

    void _process(double delta) override {
        if (render_sys && nav_sys && collision_sys) { 
            auto time = Time::get_singleton();
            auto t0 = time->get_ticks_usec();
            render_sys->update(float(delta));
            auto t1 = time->get_ticks_usec();
            nav_sys->update(float(delta));
            auto t2 = time->get_ticks_usec();
            collision_sys->update(float(delta));
            auto t3 = time->get_ticks_usec();

            print_line("render sys: ", t1-t0);
            print_line("nav sys: ", t2-t1);
            print_line("nav-collision sys: ", t3-t2);
        }
    }
    
    void _physics_process(double delta) override {
        if (physics_sys) {
            auto time = Time::get_singleton();
            auto t0 = time->get_ticks_usec();
            physics_sys->update(float(delta));
            auto t1 = time->get_ticks_usec();
            
            print_line("physics sys: ", t1-t0);
        }
    }


// Godot Boilerplate ----------------------------------
    void generate_flowfield(Vector3 _target) { nav_sys->flowfield.GenerateFlowField(_target); }

    void set_entity_count(int _count) { entity_count = _count; }
    auto get_entity_count() -> int { return entity_count; }

    void set_navmesh(Ref<NavigationMesh> _mesh) { nav_mesh = _mesh; }
    auto get_navmesh() -> Ref<NavigationMesh> { return nav_mesh; }

    void set_mesh(Ref<Mesh> _mesh) { test_mesh = _mesh; }
    auto get_mesh() -> Ref<Mesh> { return test_mesh; }

    void set_material(Ref<Material> _material) { test_material = _material; }
    auto get_material() -> Ref<Material> { return test_material; }

protected:
    static void _bind_methods() {
        ClassDB::bind_method(D_METHOD("generate_flowfield", "Vector3"), &ECSTester::generate_flowfield);

        ClassDB::bind_method(D_METHOD("set_entity_count", "_mesh"), &ECSTester::set_entity_count);
        ClassDB::bind_method(D_METHOD("get_entity_count"), &ECSTester::get_entity_count);
        ADD_PROPERTY(PropertyInfo(Variant::INT, "entity_count", PROPERTY_HINT_RESOURCE_TYPE, "int"), "set_entity_count", "get_entity_count");

        ClassDB::bind_method(D_METHOD("set_navmesh", "_mesh"), &ECSTester::set_navmesh);
        ClassDB::bind_method(D_METHOD("get_navmesh"), &ECSTester::get_navmesh);
        ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "nav_mesh", PROPERTY_HINT_RESOURCE_TYPE, "NavigationMesh"), "set_navmesh", "get_navmesh");

        ClassDB::bind_method(D_METHOD("set_mesh", "_mesh"), &ECSTester::set_mesh);
        ClassDB::bind_method(D_METHOD("get_mesh"), &ECSTester::get_mesh);
        ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "test_mesh", PROPERTY_HINT_RESOURCE_TYPE, "Mesh"), "set_mesh", "get_mesh");

        ClassDB::bind_method(D_METHOD("set_material", "_material"), &ECSTester::set_material);
        ClassDB::bind_method(D_METHOD("get_material"), &ECSTester::get_material);
        ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "test_material", PROPERTY_HINT_RESOURCE_TYPE, "Material"), "set_material", "get_material");
    }
};
