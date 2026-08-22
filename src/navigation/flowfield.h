#pragma once

#include "godot_cpp\classes\navigation_mesh.hpp"

using namespace godot;

class FlowField {
public:
	struct VectorHash {
		size_t operator()(const Vector3& _vector) const {
			return std::hash<float>{}(_vector.x) ^
					std::hash<float>{}(_vector.y) << 1 ^
					std::hash<float>{}(_vector.z) << 2;
		}
	};

	struct NavigationNode {
		NavigationNode(PackedInt32Array _polygon, Vector3 _position)
			: vertices(_polygon)
			, position(_position)
		{ }

		PackedInt32Array vertices;
		PackedInt32Array neighbour_ids;

		Vector3 position;
		Vector3 ideal_direction;

		float cost;
		bool closed;
	};

	void GenerateFlowField(const Vector3& _target);

	//void DrawCosts();
	//void DrawFlowField();

	void ResetNodes();
	void GenerateNodes(Ref<NavigationMesh> _nav_mesh);
	void GenerateCosts(const Vector3 &_target);
	void GenerateDirections(const Vector3& _target);

	auto CheckNodeChanged(const Vector3& _target, int32_t _node_id) -> int32_t;
	auto WithinNodeBounds(const Vector3& _target, int32_t _node_id, const PackedVector3Array& _vertices) -> bool;
	auto GetClosestNode(const Vector3& _target) -> int32_t;
	auto ProjectPointTri(const Vector3& _point, const Vector3& _a, const Vector3& _b, const Vector3& _c) -> Vector3;

	std::vector<NavigationNode> node_list;

	size_t target_id;
	Vector3 target_position;


	Ref<NavigationMesh> navmesh;
};
