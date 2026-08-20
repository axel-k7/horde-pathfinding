class_name FlowField
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
	var g: float = INF
	var closed: bool = false
	
	var neighbour_ids: PackedInt32Array		#neighbour node indices
	
	var best_neighbour: NavigationNode #<- change this, waste of memory
	var ideal_direction: Vector3 = Vector3.ZERO

var NODE_LIST: Array[NavigationNode] = []
var TARGET_NODE: NavigationNode = null
var TARGET_POSITION: Vector3 = Vector3.ZERO

func _ready() -> void:
	_generate_nodes()

func _process(delta: float) -> void:
	_draw_flow_field()

func generate_flow_field(_target: Vector3):
	_reset_nodes()
	TARGET_NODE = _get_closest_node(_target)
	if (TARGET_NODE == null):
		return
	
	var vs = NAVIGATION_MESH.get_vertices()
	TARGET_POSITION = _project_point_tri(_target, vs[TARGET_NODE.vertices[0]], vs[TARGET_NODE.vertices[1]], vs[TARGET_NODE.vertices[2]])
	
	_generate_costs(_target)
	_generate_directions(_target)

func query_ideal_direction(_position: Vector3) -> Vector3:
	var closest_node: NavigationNode = _get_closest_node(_position)
	if closest_node == TARGET_NODE :
		return Vector3.ZERO
	elif closest_node == null:
		return Vector3.DOWN
	
	return closest_node.ideal_direction

func _reset_nodes():
	TARGET_NODE = null
	TARGET_POSITION = Vector3.ZERO
	
	for node in NODE_LIST:
		node.g = INF
		node.closed = false
		node.best_neighbour = null
		node.ideal_direction = Vector3.ZERO

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


func _get_closest_node(_target: Vector3) -> NavigationNode:
	var vertices = NAVIGATION_MESH.get_vertices()
	for node in NODE_LIST:
		var a = vertices[node.vertices[0]]
		var b = vertices[node.vertices[1]]
		var c = vertices[node.vertices[2]]
		
		var ab = b - a
		var ac = c - a
		var bc = c - b
		var ca = a - c
		
		var normal = ab.cross(ac).normalized()
		var projected_target = _project_point_tri(_target, a, b, c) #better to calculate it manually instead here but whatever
		
		var proj_a = projected_target - a
		var proj_b = projected_target - b
		var proj_c = projected_target - c
		
		var inside_ab = ab.cross(proj_a).dot(normal) >= 0
		var inside_bc = bc.cross(proj_b).dot(normal) >= 0
		var inside_ca = ca.cross(proj_c).dot(normal) >= 0
		
		if inside_ab and inside_bc and inside_ca:
			return node
	return null


func _generate_costs(_target: Vector3):
	var target_node: NavigationNode = _get_closest_node(_target)
	debug_target = target_node
	
	if target_node == null:
		return
	
	target_node.g  = 0
	var queue: Array[NavigationNode] = [target_node]
	
	while !queue.is_empty():
		queue.sort_custom(func(_a,_b): return _a.g < _b.g)
		var current_node = queue.pop_front()
		
		if current_node.closed:
			continue
		current_node.closed = true
		
		for neighbour_id in current_node.neighbour_ids:
			var neighbour = NODE_LIST[neighbour_id]
			if neighbour.closed:
				continue
			
			var cost = current_node.g + current_node.position.distance_to(neighbour.position)
			if cost < neighbour.g:
				neighbour.g = cost
				queue.push_back(neighbour)
			
				
		
	
func _generate_directions(_target: Vector3):
	for node in NODE_LIST:
		node.closed = false
		
	var queue: Array[NavigationNode] = [TARGET_NODE]
	var vertices = NAVIGATION_MESH.get_vertices()
	
	while !queue.is_empty():
		queue.sort_custom(func(_a,_b): return _a.g < _b.g)
		var current_node: NavigationNode = queue.pop_front()
		
		if current_node == TARGET_NODE:
			current_node.closed = true
			for neighbour_id in current_node.neighbour_ids:
				var neighbour = NODE_LIST[neighbour_id]
				neighbour.ideal_direction = neighbour.position.direction_to(_target)
				queue.push_back(neighbour)
		
		if current_node.closed:
			continue
		current_node.closed = true
		
		for neighbour_id in current_node.neighbour_ids:
			var neighbour: NavigationNode = NODE_LIST[neighbour_id]
			if neighbour.closed:
				continue
			
			var furthest_vertex: Vector3 = Vector3.ZERO
			var other_verts: Array[Vector3] = []
		
			for index in neighbour.vertices:
				if current_node.vertices.has(index):
					other_verts.push_back(vertices[index])
				else:
					furthest_vertex = vertices[index] #assuming triangles right now
				
			if furthest_vertex == Vector3.ZERO:
				continue
				
			var ideal_direction: Vector3 = current_node.ideal_direction
		
			var v1_dir: Vector3 = furthest_vertex.direction_to(other_verts[0])
			var v2_dir: Vector3 = furthest_vertex.direction_to(other_verts[1])
			
			var left_bound: Vector3
			var right_bound: Vector3
			
			if v1_dir.cross(v2_dir).y > 0:
				left_bound = v1_dir
				right_bound = v2_dir
			else:
				left_bound = v2_dir
				right_bound = v1_dir
			
			var oob_left = left_bound.cross(ideal_direction).y < 0
			var oob_right = ideal_direction.cross(right_bound).y < 0
			
			if oob_left:
				neighbour.ideal_direction = left_bound
			elif oob_right:
				neighbour.ideal_direction = right_bound
			else:
				neighbour.ideal_direction = ideal_direction
				
			queue.push_back(neighbour)
	
	for node in NODE_LIST:
		if node.ideal_direction == Vector3.ZERO:
			continue
		node.ideal_direction.y = 0
		node.ideal_direction = node.ideal_direction.normalized()

func _project_point_tri(_point: Vector3, _a: Vector3, _b: Vector3, _c: Vector3) -> Vector3:
		var ab = _b - _a
		var ac = _c - _a
		var normal = ab.cross(ac).normalized()
		
		var s_dist = normal.dot(_point - _a)
		return _point - (s_dist * normal)
	
		
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
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
