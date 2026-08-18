extends NavigationRegion3D

@export var NAVIGATION_MESH: NavigationMesh
@export var debug_mesh: MeshInstance3D

var portals: Dictionary[Vector3, PackedVector3Array] = {}

var astar: AStar3D = AStar3D.new()


func _ready() -> void:
	_generate_portals(NAVIGATION_MESH)


func _process(delta: float) -> void:
	_draw_portals()

func _generate_portals(_nav_mesh: NavigationMesh):
	
	#	1st pass - find matching indices
	
	#indice -> every polygon with said indice
	var connected_indices: Dictionary[int, Array] = {} #Array[PackedInt32Array]
	
	for index: int in _nav_mesh.get_polygon_count():
		var polygon: PackedInt32Array = _nav_mesh.get_polygon(index)
		for indice in polygon:
			connected_indices.get_or_add(indice, []).append(polygon)
	
	#	2nd pass - connect polygons to eachother
	
	#polygon -> every connected polygon
	var connected_polygons: Dictionary[PackedInt32Array, Array] # Array[PackedInt32Array]
	var vertices: PackedVector3Array = _nav_mesh.get_vertices()
	
	for indice: int in connected_indices:
		for polygon: PackedInt32Array in connected_indices[indice]:
			for connected_polygon: PackedInt32Array in connected_indices[indice]:
				if polygon == connected_polygon:
					continue
				
				var value: Array[PackedInt32Array] = connected_polygons.get_or_add(polygon, [] as Array[PackedInt32Array])
				#	if polygon is already listed as a connection, create portal immediately.
				#	there can be no more than two connected vertices between convex polygons
				#	so there is no need to check for any further connections
				
				if value.has(connected_polygon):
					#check which indices are shared
					var common_indices: PackedInt32Array = []
					for poly_ind in polygon:
						if connected_polygon.has(poly_ind):
							common_indices.append(poly_ind)
							
					#portal is in the midpoint between the two indices
					portals.get_or_add(polygon, []).append((vertices[common_indices[0]] + vertices[common_indices[1]]) * 0.5)
				else:
					value.append(connected_polygon)
							
		
	#	3rd pass - create remaining portals 
	
	for polygon in connected_polygons:
		for connected_polygon in connected_polygons[polygon]:
			if connected_polygon == polygon:
				continue
			
			for poly_ind in polygon:
				if connected_polygon.has(poly_ind):
					#portal is at the indice
					portals.get_or_add(polygon, []).append(vertices[poly_ind])
					



func _draw_portals():
	var immediate_mesh = ImmediateMesh.new()
	debug_mesh.mesh = immediate_mesh
	
	var line = Vector3(0, 1, 0)
	
	for polygon in portals:
		var vertices = portals[polygon]
		
		if vertices.is_empty():
			continue
	
		immediate_mesh.surface_begin(Mesh.PRIMITIVE_LINES)
		
		for vertex in vertices:
			immediate_mesh.surface_add_vertex(vertex)
			immediate_mesh.surface_add_vertex(vertex + line)
			
		immediate_mesh.surface_end()
			
			
