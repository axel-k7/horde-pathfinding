class_name Entity
extends CharacterBody3D

var AVOIDANCE_FIELD: AvoidanceField

var id: int = -1

var move_speed: float = 0

func _ready() -> void:
	AVOIDANCE_FIELD = get_tree().get_first_node_in_group('field')
	id = AVOIDANCE_FIELD.register_entity(self)
	move_speed = randf_range(2, 10)
	
func _process(delta: float) -> void:
	var queried_dir =  AVOIDANCE_FIELD.query_ideal_direction(id)
		
	velocity = queried_dir*move_speed
	velocity.y -= 9.8
	move_and_slide()
