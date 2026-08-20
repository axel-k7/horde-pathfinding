extends NavigationRegion3D

@export var fieldwrapper: FieldWrapper
@export var debug_mesh: MeshInstance3D

func _ready() -> void:
	fieldwrapper.generate_nodes(navigation_mesh)

func _process(delta: float) -> void:
	if !debug_mesh:
		return
	fieldwrapper.draw_directions(debug_mesh)
