#pragma once

#include "godot_cpp/classes/engine.hpp"
#include "godot_cpp/classes/node3d.hpp"
#include "godot_cpp/classes/world3d.hpp"
#include "godot_cpp/classes/material.hpp"

#include "ecs/systems/RenderingSystem.h"
#include "systems/MoveTestSystem.h"
#include "ecs/core/EntityCommandBuffer.h"

using namespace godot;

class ECSTester : public Node3D {
    GDCLASS(ECSTester, Node3D)

private:
    std::shared_ptr<Registry> registry;
    std::shared_ptr<EntityCommandBuffer> buffer;

    std::unique_ptr<RenderingSystem> render_sys;
    std::unique_ptr<MoveTestSystem> move_sys;

    Ref<Mesh> test_mesh;
    Ref<Material> test_material;
    int entity_count = 50;

public:
    void _ready() override { 
        if (Engine::get_singleton()->is_editor_hint()) 
            return;

        registry = std::make_shared<Registry>();
        buffer = std::make_shared<EntityCommandBuffer>(registry);

        move_sys = std::make_unique<MoveTestSystem>(registry, buffer);

        RID world_context = get_world_3d()->get_scenario();
        render_sys = std::make_unique<RenderingSystem>(
            registry,
            buffer,
            world_context
        );


        for (int i = 0; i < entity_count; i++) {
            Entity entity = registry->allocateEntity();

            //ok to not use buffer here :) (FOR TESTING)
            Transform3D t;

            registry->addComponents<TransformComponent, MeshComponent>(entity, {}, {});

            auto transform = registry->tryGetComponent<TransformComponent>(entity);
            transform->transform.origin = Vector3(
                (rand()%20) - 10,
                (rand()%20) - 10,
                (rand()%20) - 10
            );

            auto mesh = registry->tryGetComponent<MeshComponent>(entity);
            mesh->mesh_rid = test_mesh->get_rid();
            mesh->material_rid = test_material->get_rid();
        }
    }

    void _process(double delta) override {
        if (render_sys && move_sys) { 
            render_sys->update(float(delta));
            move_sys->update(float(delta));
        }
    }


// Godot Boilerplate ----------------------------------

    void set_mesh(Ref<Mesh> _mesh) { test_mesh = _mesh; }
    auto get_mesh() -> Ref<Mesh> { return test_mesh; }

    void set_material(Ref<Material> _material) { test_material = _material; }
    auto get_material() -> Ref<Material> { return test_material; }

protected:
    static void _bind_methods() {
        ClassDB::bind_method(D_METHOD("set_mesh", "_mesh"), &ECSTester::set_mesh);
        ClassDB::bind_method(D_METHOD("get_mesh"), &ECSTester::get_mesh);
        ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "test_mesh", PROPERTY_HINT_RESOURCE_TYPE, "Mesh"), "set_mesh", "get_mesh");

        ClassDB::bind_method(D_METHOD("set_material", "_material"), &ECSTester::set_material);
        ClassDB::bind_method(D_METHOD("get_material"), &ECSTester::get_material);
        ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "test_material", PROPERTY_HINT_RESOURCE_TYPE, "Material"), "set_material", "get_material");
    }
};
