extends Node3D

@export var NAVIGATION_MESH: NavigationMesh

func _ready() -> void:	
	#generate_grid()
	#generate_grid_path(Vector3.ZERO)
	
	NavigationServer3D.map_force_update(get_world_3d().navigation_map)
	
	print(
		NavigationServer3D.map_get_path(
			get_world_3d().navigation_map,
			Vector3(0,0,-4),
			Vector3(5,0,5),
			true
		)
	)
	
	
func _process(delta: float) -> void:
	var mouse_pos = get_viewport().get_mouse_position()	
	
	#print(get_grid_direction(Vector3(mouse_pos.x, mouse_pos.y, 0))) 


#snap position to a grid aligned value
func get_region_direction(_pos: Vector3) -> Vector3:
	return Vector3.ZERO
	#var dir = grid_map.get(snapped(Vector2(_pos.x, _pos.y), Vector2(RESOLUTION, RESOLUTION)))
	#if (dir == null):
	#	return Vector3()
		
	#return dir

func generate_region_directions(_target: Vector3) -> void:
	var vertices: PackedVector3Array = NAVIGATION_MESH.get_vertices()
	
	for index: int in NAVIGATION_MESH.get_polygon_count():
		var poly_ind: PackedInt32Array = NAVIGATION_MESH.get_polygon(index)
		
		
		
