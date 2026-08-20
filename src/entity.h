#pragma once

#include "godot_cpp/classes/character_body3d.hpp"
#include "godot_cpp/classes/scene_tree.hpp"
#include "godot_cpp/classes/wrapped.hpp"
#include "godot_cpp/classes/engine.hpp"
#include "godot_cpp/classes/tree.hpp"

#include "fieldwrapper.h"

using namespace godot;

class NavigationEntity : public CharacterBody3D  {
	GDCLASS(NavigationEntity, CharacterBody3D)

protected:
	static void _bind_methods() {}

public:
	NavigationEntity() = default;
	~NavigationEntity() override {
		if (fieldwrapper && id != -1)
			fieldwrapper->unregister_entity(id);
	}

	FieldWrapper* fieldwrapper;
	int32_t id = -1;
	float move_speed = 5;

	void _ready() override {
		if (Engine::get_singleton()->is_editor_hint())
			return;

		Node* field_node = get_tree()->get_first_node_in_group("field");
		fieldwrapper = Object::cast_to<FieldWrapper>(field_node);

		if (fieldwrapper)
			id = fieldwrapper->register_entity(this);

		move_speed = UtilityFunctions::randf_range(0, 15);
	}

	void _process(double delta) override{
		if (Engine::get_singleton()->is_editor_hint())
			return;

		if (!fieldwrapper)
			return;

		Vector3 vel = fieldwrapper->query_direction(id) * move_speed;
		vel.y -= 9.8;

		set_velocity(vel);

		move_and_slide();
	}
};
