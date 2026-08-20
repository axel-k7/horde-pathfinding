#include "flowfield.h"

#include <queue>

void FlowField::ResetNodes() {
	target_id = -1;
	target_position = Vector3(0,0,0);

	for (auto& node : node_list) {
		node.closed = false;
		node.cost = FLT_MAX;
		node.ideal_direction = Vector3(0, 0, 0);
	}
}

void FlowField::GenerateNodes() {
	std::unordered_map<Vector3, std::pair<size_t, size_t>, VectorHash> edge_map = {};

	const PackedVector3Array& vertices = navmesh->get_vertices();
	const size_t& polygon_count = navmesh->get_polygon_count();

	for (int i = 0; i < polygon_count; i++) {
		const PackedInt32Array polygon = navmesh->get_polygon(i);
		NavigationNode node = NavigationNode(polygon, Vector3((vertices[polygon[0]] + vertices[polygon[1]] + vertices[polygon[2]]) / 3));

		node_list.push_back(node);

		for (int j = 0; j < 3; j++) {
			const Vector3 v1 = vertices[polygon[j]];
			const Vector3 v2 = vertices[polygon[(j+1)%3]];

			auto [it, _] = edge_map.try_emplace((v1 + v2) * 0.5f, SIZE_MAX, SIZE_MAX);

			if (it->second.first == SIZE_MAX)
				it->second.first = i;
			else
				it->second.second = i;
		}
	}
	
	for (const auto& entry : edge_map) {
		if (entry.second.first != SIZE_MAX && entry.second.second != SIZE_MAX) {
			node_list[entry.second.first].neighbour_ids.push_back(entry.second.second);
			node_list[entry.second.second].neighbour_ids.push_back(entry.second.first);
		}
	}
}

auto FlowField::GetClosestNode(const Vector3& _target) -> int32_t {
	const PackedVector3Array& vertices = navmesh->get_vertices();
	const size_t node_count = node_list.size();

	for (int i = 0; i < node_count; i++) {
		const NavigationNode &node = node_list[i];

		const Vector3& a = vertices[node.vertices[0]]; 
		const Vector3& b = vertices[node.vertices[1]]; 
		const Vector3& c = vertices[node.vertices[2]];

		const Vector3 ab = b - a;
		const Vector3 ac = c - a;
		const Vector3 bc = c - b;
		const Vector3 ca = a - c;

		const Vector3 normal = ab.cross(ac).normalized();
		const Vector3 projected_target = ProjectPointTri(_target, a, b, c);

		const Vector3 proj_a = projected_target - a;
		const Vector3 proj_b = projected_target - b;
		const Vector3 proj_c = projected_target - c;

		const bool inside_ab = ab.cross(proj_a).dot(normal) >= 0;
		const bool inside_bc = bc.cross(proj_b).dot(normal) >= 0;
		const bool inside_ca = ca.cross(proj_c).dot(normal) >= 0;

		if (inside_ab && inside_bc && inside_ca)
			return i;
	}

	return -1;
}

auto FlowField::ProjectPointTri(const Vector3& _point, const Vector3& _a, const Vector3& _b, const Vector3& _c) -> Vector3 {
	const Vector3 ab = _b - _a;
	const Vector3 ac = _c - _a;

	const Vector3 normal = ab.cross(ac).normalized();
	const auto s_dist = normal.dot(_point - _a);

	return _point - (s_dist * normal);
}


void FlowField::GenerateCosts(const Vector3 &_target) {
	if (target_id == -1)
		return;

	NavigationNode& target_node = node_list[target_id];
	target_node.cost = 0;

	auto priority = [this](size_t _a, size_t _b) {
		return this->node_list[_a].cost > this->node_list[_b].cost;
	};

	std::priority_queue<size_t, std::vector<size_t>, decltype(priority)> open_list(priority);
	open_list.push(target_id);

	while (!open_list.empty()) {
		const size_t current_id = open_list.top();
		open_list.pop();

		NavigationNode &current_node = node_list[current_id];

		if (current_node.closed)
			continue;
		current_node.closed = true;

		for (const int32_t& n_id : current_node.neighbour_ids) {
			NavigationNode &neighbour = node_list[n_id];
			if (neighbour.closed)
				continue;

			float cost = current_node.cost + current_node.position.distance_to(neighbour.position);
			if (cost < neighbour.cost) {
				neighbour.cost = cost;
				open_list.push(n_id);
			}
		}
	}

	for (auto &node : node_list) {
		node.closed = false;
	}
}

void FlowField::GenerateDirections(const Vector3& _target) {
	if (target_id == -1)
		return;

	const PackedVector3Array& vertices = navmesh->get_vertices();
	NavigationNode& target_node = node_list[target_id];
	target_node.cost = 0;

	auto priority = [this](size_t _a, size_t _b) {
		return this->node_list[_a].cost > this->node_list[_b].cost;
	};

	std::priority_queue<size_t, std::vector<size_t>, decltype(priority)> open_list(priority);
	open_list.push(target_id);

	while (!open_list.empty()) {
		const size_t current_id = open_list.top();
		open_list.pop();

		NavigationNode& current_node = node_list[current_id];

		if (current_id == target_id) {
			current_node.closed = true;
			for (const auto& n_id : current_node.neighbour_ids) {
				NavigationNode& neighbour = node_list[n_id];
				neighbour.ideal_direction = neighbour.position.direction_to(_target);
				open_list.push(n_id);
			}
			continue;
		}

		if (current_node.closed)
			continue;
		current_node.closed = true;

		for (const int32_t& n_id : current_node.neighbour_ids) {
			NavigationNode& neighbour = node_list[n_id];
			if (neighbour.closed)
				continue;

			Vector3 furthest_vertex = Vector3(0,0,0);
			PackedVector3Array other_verts;

			for (const auto index : neighbour.vertices) {
				if (current_node.vertices.has(index))
					other_verts.push_back(vertices[index]);
				else
					furthest_vertex = vertices[index];
			}

			if (furthest_vertex == Vector3(0,0,0))
				continue;

			Vector3 ideal_direction = current_node.ideal_direction;

			const Vector3 v1_dir = furthest_vertex.direction_to(other_verts[0]);
			const Vector3 v2_dir = furthest_vertex.direction_to(other_verts[1]);

			Vector3 left_bound;
			Vector3 right_bound;

			if (v1_dir.cross(v2_dir).y > 0) {
				left_bound = v1_dir;
				right_bound = v2_dir;
			} else {
				left_bound = v2_dir;
				right_bound = v1_dir;
			}

			bool oob_left = left_bound.cross(ideal_direction).y < 0;
			bool oob_right = ideal_direction.cross(right_bound).y < 0;

			if (oob_left)
				neighbour.ideal_direction = left_bound;
			else if (oob_right)
				neighbour.ideal_direction = right_bound;
			else
				neighbour.ideal_direction = ideal_direction;

			open_list.push(n_id);
		}
	}

	for (auto& node : node_list) {
		if (node.ideal_direction == Vector3(0,0,0))
			continue;

		node.ideal_direction.y = 0;
		node.ideal_direction.normalize();
	}
}


void FlowField::GenerateFlowField(const Vector3& _target) {
	ResetNodes();

	target_id = GetClosestNode(_target);
	if (target_id == -1)
		return;

	const NavigationNode& target_node = node_list[target_id];
	const PackedVector3Array& vertices = navmesh->get_vertices();
	target_position = ProjectPointTri(_target, vertices[target_node.vertices[0]], vertices[target_node.vertices[1]], vertices[target_node.vertices[2]]);

	GenerateCosts(target_position);
	GenerateDirections(target_position);
}
