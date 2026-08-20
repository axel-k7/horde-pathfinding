#pragma once

#include "flowfield.h"

class NavigationEntity;

class AvoidanceField : public FlowField {
public:
	const float AVOIDANCE_RADIUS = 2.0f;

	auto RegisterEntity(NavigationEntity* _entity) -> int32_t;
	void UnregisterEntity(int32_t _id);
	auto QueryIdealDirection(const int32_t _id) -> Vector3;


	int32_t entity_id = 0;
	std::vector<int32_t> free_ids;

	std::vector<NavigationEntity*> entities;
	std::vector<int32_t> regions;

	auto Avoidance(const int32_t _entity_id, const int32_t _region_id) -> Vector3;
};
