extends Node3D
## Everyone on the street, as simple figures until real characters arrive.
## They walk to wherever the simulation says they are, and inside a house
## they only show when it's opened up.

const WALK_SPEED := 4.0  # metres per real second on screen
const ROLE_COLOURS := [Color8(70, 96, 140), Color8(150, 70, 70), Color8(214, 170, 70), Color8(130, 120, 150)]
const FLOOR := 0.6  # house floors sit on a 0.6 m stone foundation

var _figures := {}  # Vector2i(household, person) -> {node, at}
var _ring: MeshInstance3D
var _street: Dictionary


func build(street: Dictionary) -> void:
	_street = street
	_ring = MeshInstance3D.new()
	var torus := TorusMesh.new()
	torus.inner_radius = 0.42
	torus.outer_radius = 0.55
	_ring.mesh = torus
	var mat := StandardMaterial3D.new()
	mat.albedo_color = Color8(226, 186, 96)
	mat.emission_enabled = true
	mat.emission = Color8(226, 186, 96)
	mat.emission_energy_multiplier = 0.6
	_ring.material_override = mat
	_ring.visible = false
	add_child(_ring)


func _plot(id: String) -> Dictionary:
	for p in _street.plots:
		if p.id == id:
			return p
	return {}


func _make_figure(person: Dictionary) -> Node3D:
	var child: bool = person.role == 2
	var height := 1.25 if child else 1.7
	var body := MeshInstance3D.new()
	var capsule := CapsuleMesh.new()
	capsule.radius = 0.2 if child else 0.25
	capsule.height = height
	body.mesh = capsule
	var mat := StandardMaterial3D.new()
	mat.albedo_color = ROLE_COLOURS[person.role]
	body.material_override = mat
	body.position.y = height * 0.5
	var head := MeshInstance3D.new()
	var sphere := SphereMesh.new()
	sphere.radius = 0.13 if child else 0.15
	sphere.height = sphere.radius * 2.0
	head.mesh = sphere
	var skin := StandardMaterial3D.new()
	skin.albedo_color = Color8(222, 186, 160)
	head.material_override = skin
	head.position.y = height + 0.05
	var root := Node3D.new()
	root.add_child(body)
	root.add_child(head)
	add_child(root)
	return root


## Where someone should stand for the place the simulation gives them.
func _spot(plot: Dictionary, person: Dictionary, index: int) -> Vector3:
	var spread := index * 1.6
	var house: Rect2 = plot.house
	var garden: Rect2 = plot.garden
	var wood: Rect2 = plot.woodpile
	match int(person.place):
		0:  # house
			var c := house.get_center()
			return Vector3(c.x - 2.4 + spread, FLOOR, c.y + (-1.0 if index % 2 == 0 else 1.0))
		1:  # garden
			return Vector3(garden.position.x + 2.0 + spread * 1.5, 0.0, garden.get_center().y)
		2:  # woodpile
			return Vector3(wood.position.x - 1.0, 0.0, wood.get_center().y + spread * 0.5)
		_:  # yard
			var a := house.get_center()
			var b := wood.get_center()
			var y := a.y - 8.0 if plot.north else a.y + 8.0
			return Vector3((a.x + b.x) * 0.5 + spread, 0.0, y)


func update(households: Array, houses: Node, selected: Vector2i, delta: float) -> void:
	for h in households.size():
		var household: Dictionary = households[h]
		var plot := _plot(household.plot)
		for i in household.members.size():
			var person: Dictionary = household.members[i]
			var key := Vector2i(h, i)
			if not _figures.has(key):
				_figures[key] = {"node": _make_figure(person)}
				_figures[key].node.position = _spot(plot, person, i)
			var node: Node3D = _figures[key].node
			if not person.alive:
				node.visible = false
				continue
			var target := _spot(plot, person, i)
			# Walking in or out of the house is a step up or down, not a slope.
			var flat := Vector3(target.x, node.position.y, target.z)
			node.position = node.position.move_toward(flat, WALK_SPEED * delta)
			if node.position.distance_to(flat) < 0.05:
				node.position.y = target.y
			var inside: bool = int(person.place) == 0
			node.visible = not inside or houses.is_open(houses.index_for_plot(household.plot))

	_ring.visible = false
	if _figures.has(selected):
		var node: Node3D = _figures[selected].node
		if node.visible:
			_ring.visible = true
			_ring.position = node.position + Vector3(0, 0.05, 0)


## The person nearest a ground point, within reach.
func pick(households: Array, ground: Vector3) -> Vector2i:
	var best := 1.4
	var found := Vector2i(-1, -1)
	for key in _figures:
		var node: Node3D = _figures[key].node
		if not node.visible:
			continue
		var d := Vector2(node.position.x, node.position.z).distance_to(Vector2(ground.x, ground.z))
		if d < best:
			best = d
			found = key
	return found


func position_of(key: Vector2i) -> Vector3:
	return _figures[key].node.position if _figures.has(key) else Vector3.ZERO


## Which part of a plot a ground point is on, for direct orders: 0 house,
## 1 garden, 2 woodpile, 3 yard, or -1 if it's not on the plot.
func place_at(plot_id: String, ground: Vector3) -> int:
	var plot := _plot(plot_id)
	if plot.is_empty():
		return -1
	var p := Vector2(ground.x, ground.z)
	if (plot.woodpile as Rect2).grow(1.5).has_point(p):
		return 2
	if (plot.garden as Rect2).grow(0.5).has_point(p):
		return 1
	if (plot.house as Rect2).has_point(p):
		return 0
	if (plot.bounds as Rect2).has_point(p):
		return 3
	return -1
