#pragma once


#include "avoidancefield.h"

#include "godot_cpp/classes/mesh_instance3d.hpp"
#include "godot_cpp/classes/immediate_mesh.hpp"
#include "godot_cpp/classes/ref_counted.hpp"
#include "godot_cpp/classes/wrapped.hpp"
#include "godot_cpp/classes/node.hpp"
#include "godot_cpp/variant/variant.hpp"

using namespace godot;

class FieldWrapper : public Node {
	GDCLASS(FieldWrapper, Node)

protected:
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("generate_nodes", "mesh"), &FieldWrapper::generate_nodes);
		ClassDB::bind_method(D_METHOD("generate_field", "target"), &FieldWrapper::generate_field);
		ClassDB::bind_method(D_METHOD("query_direction", "id"), &FieldWrapper::query_direction);
		ClassDB::bind_method(D_METHOD("register_entity", "entity"), &FieldWrapper::register_entity);
		ClassDB::bind_method(D_METHOD("draw_directions", "debug_mesh"), &FieldWrapper::draw_directions);
	}

	AvoidanceField AVOIDANCE_FIELD;
public:
	FieldWrapper() = default;
	~FieldWrapper() override = default;

	void generate_nodes(Ref<NavigationMesh> _mesh) {
		AVOIDANCE_FIELD.GenerateNodes(_mesh);
	}

	void generate_field(const Vector3& _target) {
		AVOIDANCE_FIELD.GenerateFlowField(_target);
	}

	auto query_direction(const int32_t _id) -> Vector3 {
		return AVOIDANCE_FIELD.QueryIdealDirection(_id);
	}

	auto register_entity(NavigationEntity* _entity) -> int32_t {
		return AVOIDANCE_FIELD.RegisterEntity(_entity);
	}

	auto unregister_entity(int32_t _id) {
		return AVOIDANCE_FIELD.UnregisterEntity(_id);
	}

	void draw_directions(MeshInstance3D* _debug_mesh) {
		Ref<ImmediateMesh> imm_mesh;
		imm_mesh.instantiate();
		_debug_mesh->set_mesh(imm_mesh);

		const auto &nodes = AVOIDANCE_FIELD.node_list;

		imm_mesh->surface_begin(Mesh::PRIMITIVE_TRIANGLES);

		for (int i = 0; i < nodes.size(); i++) {
			const auto& node = nodes[i];
			Vector3 right = node.ideal_direction.cross(Vector3(0, 1, 0)).normalized();

			imm_mesh->surface_add_vertex(Vector3(0, 1, 0) + node.position + right * 0.3f);
			imm_mesh->surface_add_vertex(Vector3(0, 1, 0) + node.position - right * 0.3f);
			imm_mesh->surface_add_vertex(Vector3(0, 1, 0) + node.position + node.ideal_direction);
		}

		imm_mesh->surface_end();
	}
};
