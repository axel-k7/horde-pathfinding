#pragma once

#include "godot_cpp/variant/vector3.hpp"

using namespace godot;

struct NavigationComponent {
    NavigationComponent(float _move_speed)
        : move_speed(_move_speed) 
    {}

    Vector3 direction = Vector3(0,0,0);
    float move_speed;

    int32_t region_id = -1;
};