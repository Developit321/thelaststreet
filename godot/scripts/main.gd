extends Node3D
## The Last Street in 3D. Runs the C++ simulation (through StreetSim) and
## builds the world, houses, people, camera and interface around it.

const World := preload("res://godot/scripts/world.gd")
const Houses := preload("res://godot/scripts/houses.gd")
const People := preload("res://godot/scripts/people.gd")
const CameraRig := preload("res://godot/scripts/camera_rig.gd")
const Hud := preload("res://godot/scripts/hud.gd")
const MAX_STEPS_PER_FRAME := 2000

var sim  # StreetSim, from the C++ extension
var street: Dictionary
var player := -1
var paused := false
var speed_index := 0
var selected := Vector2i(-1, -1)
var following := false

var _speeds := [1.0]
var _seconds_per_day := 60.0
var _pending := 0.0
var _world: Node3D
var _houses: Node3D
var _people: Node3D
var _camera: Node3D
var _hud: CanvasLayer
var _households: Array = []


func _ready() -> void:
	if not ClassDB.class_exists("StreetSim"):
		_fail("The simulation library isn't built. Run `make godot` in the project folder.")
		return
	sim = ClassDB.instantiate("StreetSim")
	if not sim.load(ProjectSettings.globalize_path("res://data")):
		_fail("The game data could not be loaded:\n" + sim.get_error())
		return
	_read_pace()

	street = sim.get_street()
	player = sim.player_household()
	_households = sim.get_households()

	_houses = Houses.new()
	add_child(_houses)
	_houses.build(street)
	_world = World.new()
	add_child(_world)
	_world.build(street, _houses.wood_material())
	_people = People.new()
	add_child(_people)
	_people.build(street)

	_camera = CameraRig.new()
	_camera.name = "CameraRig"
	add_child(_camera)
	var area: Rect2 = street.plots[0].bounds
	for p in street.plots:
		area = area.merge(p.bounds)
	# Looking along the street from the east, like the concept art.
	_camera.frame(Vector3(area.get_center().x, 0, area.get_center().y), 78.0, deg_to_rad(62.0))
	_camera.clicked.connect(_on_ground_clicked)

	_hud = Hud.new()
	add_child(_hud)
	_hud.setup(self, _households, player, _speeds)

	# Screenshot mode: Godot --path . -- --capture=<folder> [--shots=a,b]
	var folder := ""
	var shots := []
	for arg in OS.get_cmdline_user_args():
		if arg.begins_with("--capture="):
			folder = arg.trim_prefix("--capture=")
		elif arg.begins_with("--shots="):
			shots = Array(arg.trim_prefix("--shots=").split(","))
	if folder != "":
		_hud.visible = false
		var capture := preload("res://godot/scripts/capture.gd").new()
		add_child(capture)
		capture.run(self, folder, shots)


func _read_pace() -> void:
	var text := FileAccess.get_file_as_string("res://data/calendar.json")
	var data = JSON.parse_string(text)
	if data is Dictionary and data.has("pace"):
		_seconds_per_day = float(data.pace.real_seconds_per_day)
		_speeds = data.pace.speeds


func _fail(message: String) -> void:
	var label := Label.new()
	label.text = message
	label.position = Vector2(40, 40)
	label.add_theme_font_size_override("font_size", 20)
	var layer := CanvasLayer.new()
	layer.add_child(label)
	add_child(layer)
	push_error(message)


func _process(delta: float) -> void:
	if sim == null or street.is_empty():
		return

	if not paused and not sim.player_lost():
		_pending += delta * 1440.0 / _seconds_per_day * float(_speeds[speed_index])
		var steps := int(_pending)
		if steps > 0:
			_pending -= steps
			sim.advance(mini(steps, MAX_STEPS_PER_FRAME))

	_households = sim.get_households()
	var events: Array = sim.take_events()
	if not events.is_empty():
		_hud.add_events(events, _households, func(m): return sim.date_at(m))

	var time: Dictionary = sim.get_time()
	_world.update_sky(time)
	_world.update_season(time)
	_world.update_households(_households, time)
	_world.follow_view(_camera.target)
	_update_cutaways()
	_houses.update(_households, time)
	_people.update(_households, _houses, selected, delta)
	if following and selected.x >= 0:
		_camera.target = _camera.target.lerp(_people.position_of(selected), minf(1.0, delta * 4.0))
	_hud.update(time, _households, sim.get_warnings(player))


## Houses open up (roof off, walls low) when you zoom in close to them, or
## when someone inside is selected.
func _update_cutaways() -> void:
	var close: bool = _camera.distance < 45.0
	var selected_house := -1
	if selected.x >= 0 and int(_households[selected.x].members[selected.y].place) == 0:
		selected_house = _houses.index_for_plot(_households[selected.x].plot)
	for i in _houses.houses.size():
		var plot: Dictionary = _houses.houses[i].plot
		var c: Vector2 = (plot.house as Rect2).get_center()
		var near: bool = Vector2(_camera.target.x, _camera.target.z).distance_to(c) < 12.0
		_houses.set_open(i, (close and near) or (i == selected_house and _camera.distance < 80.0))


func _unhandled_input(event: InputEvent) -> void:
	if not (event is InputEventKey and event.pressed and not event.echo):
		return
	match (event as InputEventKey).keycode:
		KEY_SPACE: set_paused(not paused)
		KEY_1: set_speed(0)
		KEY_2: set_speed(1)
		KEY_3: set_speed(2)
		KEY_4: set_speed(3)
		KEY_R: _camera.reset()
		KEY_QUOTELEFT, KEY_F1: _hud.toggle_debug()
		KEY_ESCAPE: following = false


func _on_ground_clicked(ground: Vector3) -> void:
	var who: Vector2i = _people.pick(_households, ground)
	if who.x >= 0:
		select(who)
		return
	# With one of your family selected, clicking your plot sends them there.
	if selected.x == player and _households[player].members[selected.y].alive:
		var place: int = _people.place_at(_households[player].plot, ground)
		if place >= 0:
			sim.order(player, selected.y, place)
			return
	select(Vector2i(-1, -1))


# --- Called by the interface -----------------------------------------------

func select(who: Vector2i) -> void:
	selected = who
	if who.x < 0:
		following = false


func set_paused(value: bool) -> void:
	paused = value


func set_speed(index: int) -> void:
	if index >= 0 and index < _speeds.size():
		speed_index = index
		paused = false


func set_job(person: int, job: int) -> void:
	sim.set_job(player, person, job)


func cancel_order() -> void:
	if selected.x >= 0:
		sim.cancel_order(selected.x, selected.y)


func empty_store(food: bool) -> void:
	var h: Dictionary = _households[player]
	if food:
		sim.adjust_stores(player, -(h.fresh_food + h.preserved_food), 0.0)
	else:
		sim.adjust_stores(player, 0.0, -h.firewood)


func toggle_gi() -> void:
	_world.environment.sdfgi_enabled = not _world.environment.sdfgi_enabled


func restart() -> void:
	sim.restart()
	_hud.clear_log()
	select(Vector2i(-1, -1))
	paused = false
