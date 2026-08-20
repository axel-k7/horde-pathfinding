extends Control

@export var spawner: EntitySpawner

@onready var counter: Label = $VBoxContainer/entity_count

func _on_spawn_100_pressed() -> void:
	spawner.birth_children(100)
	
func _on_remove_100_pressed() -> void:
	spawner.kill_children(100)
	
func _ready() -> void:
	_update_counter(null)
	spawner.child_entered_tree.connect(_update_counter)
	spawner.child_exiting_tree.connect(_update_counter)

func _update_counter(node: Node):
	counter.text = "child amount: " + str(spawner.get_child_count()); 
