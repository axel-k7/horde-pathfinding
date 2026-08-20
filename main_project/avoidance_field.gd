class_name AvoidanceField
extends FlowField

const AVOIDANCE_RADIUS: float = 2.0

var ENTITY_ID: int = 0
var FREE_IDS: Array[int] #packed int doesnt have pop back

var ENTITIES: Array[Entity]
var REGIONS: Array[int]  #entity index maps to node list index


func register_entity(_entity: Entity) -> int:
	var id = -1
	if FREE_IDS.is_empty():
		id = ENTITY_ID
		ENTITY_ID += 1
	else:
		id = FREE_IDS.pop_back()
	
	if ENTITIES.size() < id+1:
		ENTITIES.resize(id+1)
		REGIONS.resize(id+1)
		
	ENTITIES[id] = _entity
	REGIONS[id] = _get_closest_node(_entity.position)
	
	return id

func query_ideal_direction(_id: int) -> Vector3:
	var region_id = _get_closest_node(ENTITIES[_id].position)
	if region_id != REGIONS[_id]:
		REGIONS[_id] = region_id

	#return NODE_LIST[region_id].ideal_direction
	return avoidance(_id, region_id)	

func avoidance(_entity_id: int, _region_id) -> Vector3:
	#yuck, checks every single registered entity
	var node = NODE_LIST[_region_id]
	var entity = ENTITIES[_entity_id]
	var dir = node.ideal_direction
	
	if node == TARGET_NODE:
		if entity.position.distance_to(TARGET_POSITION) > 0.1:
			return entity.position.direction_to(TARGET_POSITION)
		else:
			return Vector3.ZERO
	elif node == null:
		return Vector3.DOWN
	
	for id in ENTITIES.size():
		if id != _entity_id && _region_id == REGIONS[_entity_id]:
			var other_entity = ENTITIES[id]
			var predicted_pos = entity.position + node.ideal_direction*entity.move_speed
			var dist = predicted_pos.distance_to(other_entity.position)
			if dist < AVOIDANCE_RADIUS:
				var to_other: Vector3 = (entity.position - other_entity.position).normalized()
				var right: Vector3 = to_other.cross(Vector3.UP).normalized()
				
				var right_dot = node.ideal_direction.dot(right)
				
				var avoidance_dir = right if right_dot > 0 else -right
				
				var avoidance_str = (AVOIDANCE_RADIUS - dist) / AVOIDANCE_RADIUS
				
				dir += avoidance_dir * avoidance_str
				dir = dir.normalized()
				
	return dir
