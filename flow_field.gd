extends NavigationRegion3D

#the given navigation mesh is only used as for mesh degeneration
@export var NAVIGATION_MESH: NavigationMesh

@export var debug: MeshInstance3D
var debug_target: NavigationNode

class Portal:
	func _init(_position: Vector3) -> void:
		position = _position
		
	var position: Vector3
	var connected_polygons: PackedInt32Array = []

class NavigationNode:
	var vertices: PackedInt32Array		#indices
	var position: Vector3				#middle-point of the node
	var g: float
	var closed: bool = false
	
	var neighbour_ids: PackedInt32Array		#neighbour node indices
	
	var ideal_direction: Vector3 = Vector3.ZERO

var NODE_LIST: Array[NavigationNode] = []

func _ready() -> void:
	_generate_nodes()
	generate_flow_field(Vector3.ZERO)
	pass

func _process(delta: float) -> void:
	_draw_flow_field()

func generate_flow_field(_target: Vector3):
	_generate_costs(_target)
	_generate_directions()

func get_ideal_direction(_target: Vector3) -> Vector3:
	return NODE_LIST[_get_closest_point(_target)].ideal_direction


func _generate_nodes():
	var edge_map: Dictionary[Vector3, PackedInt32Array] = {}
	
	var vertices: PackedVector3Array = NAVIGATION_MESH.get_vertices()
	for index: int in NAVIGATION_MESH.get_polygon_count():
		var new_node = NavigationNode.new()
		var polygon: PackedInt32Array = NAVIGATION_MESH.get_polygon(index)
		new_node.vertices = polygon
		new_node.position = (vertices[polygon[0]] + vertices[polygon[1]] + vertices[polygon[2]]) / 3
		
		NODE_LIST.push_back(new_node)
		
		for indice: int in polygon.size():
			var v1 = vertices[polygon[indice]]
			var v2 = vertices[polygon[indice+1]] if indice != polygon.size()-1 else vertices[polygon[0]]
			
			var edge = edge_map.get_or_add((v1 + v2) * 0.5, [INT32_MAX, INT32_MAX])
			
			if edge[0] == INT32_MAX:
				edge[0] = index
			else:
				edge[1] = index
		
	for entry: Vector3 in edge_map:
		var connected_pair: PackedInt32Array = edge_map[entry]
		if connected_pair[0] != INT32_MAX and connected_pair[1] != INT32_MAX:
			var node1: NavigationNode = NODE_LIST[connected_pair[0]]
			var node2: NavigationNode = NODE_LIST[connected_pair[1]]

			node1.neighbour_ids.push_back(connected_pair[1])
			node2.neighbour_ids.push_back(connected_pair[0])
			
		

func _get_closest_point(_target: Vector3) -> int:
	var closest_node_id: int = 0
	var closest_dist: float = INF
	
	for id in NODE_LIST.size():
		var curr_dist: float = NODE_LIST[id].position.distance_to(_target)
		if curr_dist < closest_dist:
			closest_dist = curr_dist
			closest_node_id = id
	
	return closest_node_id

func _generate_costs(_target: Vector3):
	var target_node: NavigationNode = NODE_LIST[_get_closest_point(_target)]
	debug_target = target_node
	var open_nodes: Array[NavigationNode] = [target_node]
	
	var cost: float = 1.0
	
	while !open_nodes.is_empty():
		cost += 1.0
		var current_node = open_nodes[0]
		open_nodes.erase(current_node)
		
		if current_node.closed:
			continue
		
		current_node.closed = true
		current_node.g = cost + current_node.position.distance_to(target_node.position)
		
		for curr_neighbour_id in current_node.neighbour_ids:
			var current_neighbour = NODE_LIST[curr_neighbour_id]
			if current_neighbour.closed:
				continue
			open_nodes.push_back(current_neighbour)
		
	
func _generate_directions():
	for current_node in NODE_LIST:
		var best_neighbour: NavigationNode
		var lowest_cost: int = INT32_MAX
		
		for neighbour_id in current_node.neighbour_ids:
			var neighbour_node = NODE_LIST[neighbour_id]
			if neighbour_node.g < lowest_cost:
				lowest_cost = neighbour_node.g
				best_neighbour = neighbour_node
		
		if best_neighbour != null:
			current_node.ideal_direction = current_node.position.direction_to(best_neighbour.position)
		
			
		
func _draw_costs():
	var imm_mesh = ImmediateMesh.new()
	debug.mesh = imm_mesh
	
	imm_mesh.surface_begin(Mesh.PRIMITIVE_LINES)
	
	for node in NODE_LIST:
		imm_mesh.surface_add_vertex(node.position)
		imm_mesh.surface_add_vertex(node.position + Vector3(0,node.g,0))
		
	imm_mesh.surface_end()
	
func _draw_flow_field():
	var imm_mesh = ImmediateMesh.new()
	debug.mesh = imm_mesh
	
	imm_mesh.surface_begin(Mesh.PRIMITIVE_TRIANGLES)
	
	for node in NODE_LIST:
		if (node == debug_target):
			continue
			
		var right = node.ideal_direction.cross(Vector3.UP).normalized()
		
		imm_mesh.surface_add_vertex(Vector3.UP + node.position + right*0.3)
		imm_mesh.surface_add_vertex(Vector3.UP + node.position - right*0.3)
		imm_mesh.surface_add_vertex(Vector3.UP + node.position + node.ideal_direction)
		
	imm_mesh.surface_end()
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
