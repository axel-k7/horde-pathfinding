extends Node3D

@export var camera: Camera3D
@export var ecs_test: ECSTester

func _ready() -> void:
	pass # Replace with function body.

func _input(event: InputEvent) -> void:
	if event is InputEventMouseButton:
		if event.button_index == MouseButton.MOUSE_BUTTON_LEFT:
			var mouse_pos = get_viewport().get_mouse_position()
			ecs_test.generate_flowfield(
				camera.project_ray_origin(mouse_pos) + 
				camera.project_ray_normal(mouse_pos) *
				80			
			)
			
			
