#include "avoidancefield.h"

#include "navigationentity.h"

auto AvoidanceField::RegisterEntity(NavigationEntity* _entity) -> int32_t {
	int id = -1;
	if (free_ids.empty())
		id = entity_id++;
	else {
		id = free_ids.back();
		free_ids.pop_back();
	}

	if (entities.size() < id + 1) {
		entities.resize(id + 1);
		regions.resize(id + 1);
	}

	entities[id] = _entity;
	regions[id] = GetClosestNode(_entity->get_global_position());

	return id;
}

void AvoidanceField::UnregisterEntity(int32_t _id) {
	entities[_id] = nullptr;
	regions[_id] = -1;

	free_ids.push_back(_id);
}

auto AvoidanceField::QueryIdealDirection(const int32_t _id) -> Vector3 {
	const int32_t region_id = GetClosestNode(entities[_id]->get_global_position());
	if (region_id != regions[_id])
		regions[_id] = region_id;

	return Avoidance(_id, region_id);
}

auto AvoidanceField::Avoidance(int32_t _entity_id, int32_t _region_id) -> Vector3 {
	const NavigationNode& node = node_list[_region_id];
	NavigationEntity* entity = entities[_entity_id];

	const Vector3& entity_pos = entity->get_global_position();

	Vector3 direction = node.ideal_direction;

	if (_region_id == target_id) {
		if (entity_pos.distance_to(target_position) > 0.1)
			return entity_pos.direction_to(target_position);
		else
			return Vector3(0,0,0);
	}

	for (int32_t other_id = 0; other_id < entities.size(); other_id++) {
		if (other_id == _entity_id || _region_id != regions[other_id] || regions[other_id] == -1)
			continue;

		NavigationEntity* other_entity = entities[other_id];
		if (!other_entity)
			continue;

		const Vector3& other_pos = other_entity->get_global_position();

		const Vector3 predicted_pos = entity_pos + node.ideal_direction * entity->move_speed;
		const auto dist = predicted_pos.distance_to(other_pos);

		if (dist < AVOIDANCE_RADIUS) {
			const Vector3 to_other = (entity_pos - other_pos).normalized();
			const Vector3 right = to_other.cross(Vector3(0, 1, 0)).normalized();

			const auto right_dot = node.ideal_direction.dot(right);

			const Vector3 avoidance_dir = right_dot > 0 ? right : -right;
			const float avoidance_str = (AVOIDANCE_RADIUS - dist) / AVOIDANCE_RADIUS;

			direction += avoidance_dir * avoidance_str;
			direction.normalize();
		}
	}

	return direction;
}
