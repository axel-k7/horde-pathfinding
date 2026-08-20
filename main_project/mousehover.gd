extends Node3D

@export var camera: Camera3D
@export var flowfield: FlowField

func _ready() -> void:
	pass # Replace with function body.

func _input(event: InputEvent) -> void:
	if event is InputEventMouseButton:
		if event.button_index == MouseButton.MOUSE_BUTTON_LEFT:
			var mouse_pos = get_viewport().get_mouse_position()
			flowfield.generate_flow_field(
				camera.project_ray_origin(mouse_pos) + 
				camera.project_ray_normal(mouse_pos) *
				80			
			)
			
			
