class_name Entity
extends CharacterBody3D

var FLOWFIELD: FlowField

func _ready() -> void:
	FLOWFIELD = get_tree().get_first_node_in_group('field')
	
func _process(delta: float) -> void:
	var queried_dir =  FLOWFIELD.query_ideal_direction(global_position)
	
	if queried_dir == Vector3.ZERO && FLOWFIELD.TARGET_NODE != null:
		var rand = randf_range(0.5, 1.5)
		position = FLOWFIELD.TARGET_POSITION + Vector3(rand, rand, rand)
		
	velocity = queried_dir*10
	move_and_slide()
