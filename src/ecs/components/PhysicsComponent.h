#pragma once

#include "godot_cpp/classes/mesh.hpp"

using namespace godot;

struct PhysicsComponent {
    RID body_rid;
    RID shape_rid;

    int collision_layer;
};