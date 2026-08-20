#extends NavigationEntity
#
#var FIELDWRAPPER: FieldWrapper
#
#var id: int
#var move_speed: float
#
#func _ready() -> void:
#	FIELDWRAPPER = get_tree().get_first_node_in_group('field')
#	id = FIELDWRAPPER.register_entity(self)
#	move_speed = randf_range(2, 10)
#	
#func _process(delta: float) -> void:
#	var queried_dir =  FIELDWRAPPER.query_direction(id)
#		
#	velocity = queried_dir*move_speed
#	velocity.y -= 9.8
#	move_and_slide()
#
