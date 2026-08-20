#pragma once

#include "godot_cpp/classes/mesh.hpp"

using namespace godot;

struct MeshComponent {
    RID multimesh_rid; //set by rendering system

    RID mesh_rid;
    RID material_rid;
    int bucket_index = -1;
};