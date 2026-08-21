#pragma once

#include "godot_cpp/variant/vector3.hpp"

using namespace godot;

struct VelocityComponent {
    Vector3 velocity = Vector3(0,0,0);
    Vector3 acceleration = Vector3(0,0,0);
};