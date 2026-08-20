class_name EntitySpawner
extends Node3D

@onready var entity = preload("res://entity.tscn")

func _ready() -> void:
	birth_children(1000)

func birth_children(_num: int):
	for i in _num:
		var child = entity.instantiate();
		child.position += Vector3(randi_range(-45, 45), 10, randi_range(-45, 45))
		add_child(child)
		
func kill_children(_num: int):
	var living_children = get_children().filter(
		func(child): return not child.is_queued_for_deletion()
	)
	
	if _num > living_children.size():
		_num = living_children.size()
	
	for i in _num:
		living_children[i].queue_free()	
