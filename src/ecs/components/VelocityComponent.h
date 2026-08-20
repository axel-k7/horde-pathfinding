#pragma once

#include "godot_cpp/variant/vector3.hpp"

using namespace godot;

struct VelocityComponent {
    Vector3 velocity;
    Vector3 acceleration;
};