extends Node3D
## The street's surroundings: ground, road, fences, gardens, woodpiles, the
## pump, the railway and the forest, plus the sun, sky, fog and snowfall.
## Visual only: everything it shows comes from the simulation.
##
## Street coordinates are metres with x east and y south; in Godot that's
## x and z, with y up.

const SEASONS := 4

# Colours at the middle of each season (spring, summer, autumn, winter before
# snow), matching the 2D prototype. Between them the world blends smoothly.
const PALETTE := {
	"forest": [Color8(52, 62, 42), Color8(46, 64, 40), Color8(78, 66, 42), Color8(60, 62, 56)],
	"clearing": [Color8(110, 122, 74), Color8(104, 128, 72), Color8(148, 130, 78), Color8(136, 128, 104)],
	"yard": [Color8(104, 118, 70), Color8(116, 132, 78), Color8(138, 122, 76), Color8(128, 120, 98)],
	"road": [Color8(98, 78, 58), Color8(122, 100, 72), Color8(108, 86, 62), Color8(102, 90, 78)],
	"birch": [Color8(132, 156, 96), Color8(96, 124, 66), Color8(212, 152, 52), Color8(122, 110, 98)],
}
const SNOW := Color(0.91, 0.93, 0.95)
const PINE := Color8(34, 58, 38)

var sun: DirectionalLight3D
var moon: DirectionalLight3D
var environment: Environment
var sky_material: ProceduralSkyMaterial
var snowfall: GPUParticles3D

var _mats := {}
var _gardens := []      # one entry per household: {multimesh, material, total}
var _woodpiles := []    # one entry per household: {multimesh, capacity}
var _birch_material: StandardMaterial3D
var _pine_material: StandardMaterial3D


static func to3(p: Vector2, y := 0.0) -> Vector3:
	return Vector3(p.x, y, p.y)


static func rect_center(r: Rect2, y := 0.0) -> Vector3:
	return Vector3(r.position.x + r.size.x * 0.5, y, r.position.y + r.size.y * 0.5)


func build(street: Dictionary, households: Array, wood_material: Material) -> void:
	_build_light_and_sky()
	_build_ground(street)
	_build_railway(street)
	_build_pump(street, wood_material)
	for plot in street.plots:
		_build_fence(plot, wood_material)
	for h in households:
		var plot := _plot_for(street, h.plot)
		_gardens.append(_build_garden(plot))
		_woodpiles.append(_build_woodpile(plot, wood_material))
	_build_forest(street)
	_build_snowfall()


func _plot_for(street: Dictionary, id: String) -> Dictionary:
	for p in street.plots:
		if p.id == id:
			return p
	return {}


# --- Light, sky, fog -------------------------------------------------------

func _build_light_and_sky() -> void:
	sky_material = ProceduralSkyMaterial.new()
	sky_material.sky_curve = 0.12
	sky_material.ground_bottom_color = Color(0.18, 0.2, 0.16)
	var sky := Sky.new()
	sky.sky_material = sky_material

	environment = Environment.new()
	environment.background_mode = Environment.BG_SKY
	environment.sky = sky
	environment.ambient_light_source = Environment.AMBIENT_SOURCE_SKY
	environment.tonemap_mode = Environment.TONE_MAPPER_AGX
	environment.ssao_enabled = true
	environment.ssao_radius = 1.2
	environment.ssao_intensity = 2.0
	environment.glow_enabled = true
	environment.glow_intensity = 0.4
	environment.fog_enabled = true
	environment.fog_density = 0.003
	environment.fog_aerial_perspective = 0.6
	var world_env := WorldEnvironment.new()
	world_env.environment = environment
	add_child(world_env)

	sun = DirectionalLight3D.new()
	sun.shadow_enabled = true
	sun.directional_shadow_max_distance = 260.0
	sun.light_angular_distance = 0.8
	add_child(sun)

	moon = DirectionalLight3D.new()
	moon.light_color = Color(0.55, 0.65, 1.0)
	moon.shadow_enabled = true
	moon.rotation_degrees = Vector3(-50, 30, 0)
	add_child(moon)


## Moves the sun and moon, colours the sky and sets the fog for this moment.
func update_sky(time: Dictionary) -> void:
	var hour: float = time.hour + time.minute / 60.0
	var sunrise: float = time.sunrise
	var sunset: float = time.sunset
	var light: float = time.light
	var day_length := sunset - sunrise
	# Long summer days bring the sun high; short winter days keep it low.
	var max_elevation := deg_to_rad(10.0 + clampf((day_length - 6.0) / 12.0, 0.0, 1.0) * 46.0)
	var t := clampf((hour - sunrise) / day_length, 0.0, 1.0)
	var elevation := sin(PI * t) * max_elevation
	var azimuth := lerpf(deg_to_rad(-105), deg_to_rad(105), t)  # east, through south, to west
	var towards_sun := Vector3(sin(azimuth) * cos(elevation), sin(elevation), cos(azimuth) * cos(elevation))
	if towards_sun.y > 0.01:
		sun.look_at_from_position(towards_sun * 100.0, Vector3.ZERO, Vector3.UP)
	var low := 1.0 - clampf(elevation / deg_to_rad(25.0), 0.0, 1.0)
	sun.light_color = Color(1.0, 0.95, 0.88).lerp(Color(1.0, 0.68, 0.42), low)
	sun.light_energy = 1.25 * light
	sun.visible = light > 0.01
	moon.light_energy = 0.14 * (1.0 - light)
	moon.visible = light < 0.99

	var day_top := Color(0.36, 0.5, 0.72)
	var day_horizon := Color(0.72, 0.74, 0.74).lerp(Color(0.95, 0.72, 0.5), low * 0.7)
	var night_top := Color(0.02, 0.03, 0.07)
	var night_horizon := Color(0.05, 0.07, 0.12)
	sky_material.sky_top_color = night_top.lerp(day_top, light)
	sky_material.sky_horizon_color = night_horizon.lerp(day_horizon, light)
	sky_material.ground_horizon_color = sky_material.sky_horizon_color
	environment.ambient_light_energy = lerpf(0.15, 0.8, light)

	var snowing: bool = time.snowing
	var winter := float(time.season == 3)
	environment.fog_density = 0.003 + winter * 0.003 + (0.01 if snowing else 0.0)
	environment.fog_light_color = sky_material.sky_horizon_color
	snowfall.emitting = snowing


# --- Ground and road -------------------------------------------------------

func _flat(name: String, rect: Rect2, y: float, material: Material) -> MeshInstance3D:
	var mesh := PlaneMesh.new()
	mesh.size = rect.size
	var mi := MeshInstance3D.new()
	mi.name = name
	mi.mesh = mesh
	mi.material_override = material
	mi.position = rect_center(rect, y)
	add_child(mi)
	return mi


func _ground_material(seed_value: int, scale: float) -> StandardMaterial3D:
	var noise := FastNoiseLite.new()
	noise.seed = seed_value
	noise.frequency = 0.02
	noise.fractal_octaves = 5
	var tex := NoiseTexture2D.new()
	tex.noise = noise
	tex.seamless = true
	tex.width = 512
	tex.height = 512
	var ramp := Gradient.new()
	ramp.set_color(0, Color(0.72, 0.72, 0.72))
	ramp.set_color(1, Color(1.0, 1.0, 1.0))
	tex.color_ramp = ramp
	var m := StandardMaterial3D.new()
	m.albedo_texture = tex
	m.uv1_triplanar = true
	m.uv1_world_triplanar = true
	m.uv1_scale = Vector3.ONE / scale
	m.roughness = 1.0
	return m


func _build_ground(street: Dictionary) -> void:
	var world_rect: Rect2 = street.world
	_mats.forest = _ground_material(1, 30.0)
	_mats.clearing = _ground_material(2, 18.0)
	_mats.yard = _ground_material(3, 10.0)
	_mats.road = _ground_material(4, 6.0)
	_mats.rut = _ground_material(5, 4.0)
	_flat("Forest floor", world_rect.grow(200.0), -0.02, _mats.forest)

	# The clearing the street sits in: every plot plus a margin.
	var area: Rect2 = street.plots[0].bounds
	for p in street.plots:
		area = area.merge(p.bounds)
	_flat("Clearing", area.grow(8.0), 0.0, _mats.clearing)

	var road: Rect2 = street.road
	_flat("Road", Rect2(road.position.x - 200.0, road.position.y, road.size.x + 400.0, road.size.y), 0.02, _mats.road)
	for f in [0.3, 0.7]:
		var y: float = road.position.y + road.size.y * f
		_flat("Rut", Rect2(road.position.x - 200.0, y - 0.25, road.size.x + 400.0, 0.5), 0.025, _mats.rut)

	for p in street.plots:
		_flat("Yard", p.bounds, 0.01, _mats.yard)


## Season and snow colours for the ground, trees and gardens.
func update_season(time: Dictionary) -> void:
	var pos: float = time.season + time.season_progress - 0.5
	if pos < 0.0:
		pos += SEASONS
	var from := int(floorf(pos)) % SEASONS
	var to := (from + 1) % SEASONS
	var t := pos - floorf(pos)
	var snow: float = time.snow

	var pick := func(key: String) -> Color:
		return (PALETTE[key][from] as Color).lerp(PALETTE[key][to], t)
	_mats.forest.albedo_color = pick.call("forest").lerp(SNOW, snow * 0.85)
	_mats.clearing.albedo_color = pick.call("clearing").lerp(SNOW, snow)
	_mats.yard.albedo_color = pick.call("yard").lerp(SNOW, snow)
	var road: Color = pick.call("road")
	_mats.road.albedo_color = road.lerp(SNOW, snow * 0.75)
	_mats.rut.albedo_color = road.darkened(0.25).lerp(SNOW, snow * 0.4)
	_birch_material.albedo_color = pick.call("birch").lerp(SNOW, snow * 0.5)
	_pine_material.albedo_color = PINE.lerp(Color(0.7, 0.75, 0.76), snow * 0.45)


# --- Fences ----------------------------------------------------------------

func _build_fence(plot: Dictionary, wood: Material) -> void:
	var b: Rect2 = plot.bounds
	var gate_x: float = (plot.house as Rect2).get_center().x
	var front_y := b.end.y if plot.north else b.position.y
	var back_y := b.position.y if plot.north else b.end.y
	# Each side as (from, to); the front has a gate gap.
	var sides := [
		[Vector2(b.position.x, back_y), Vector2(b.end.x, back_y)],
		[Vector2(b.position.x, b.position.y), Vector2(b.position.x, b.end.y)],
		[Vector2(b.end.x, b.position.y), Vector2(b.end.x, b.end.y)],
		[Vector2(b.position.x, front_y), Vector2(gate_x - 1.5, front_y)],
		[Vector2(gate_x + 1.5, front_y), Vector2(b.end.x, front_y)],
	]
	var pickets := []
	for s in sides:
		var a: Vector2 = s[0]
		var c: Vector2 = s[1]
		var length := a.distance_to(c)
		var dir := (c - a).normalized()
		var n := int(length / 0.16)
		for i in n:
			pickets.append([a + dir * (i + 0.5) * 0.16, dir])
		# Two rails.
		for h in [0.35, 0.85]:
			var rail := MeshInstance3D.new()
			var box := BoxMesh.new()
			box.size = Vector3(length, 0.07, 0.05)
			rail.mesh = box
			rail.material_override = wood
			rail.position = to3((a + c) * 0.5, h)
			rail.rotation.y = -atan2(dir.y, dir.x)
			add_child(rail)

	var mm := MultiMesh.new()
	mm.transform_format = MultiMesh.TRANSFORM_3D
	var picket := BoxMesh.new()
	picket.size = Vector3(0.09, 1.0, 0.03)
	mm.mesh = picket
	mm.instance_count = pickets.size()
	var rng := RandomNumberGenerator.new()
	rng.seed = hash(plot.id)
	for i in pickets.size():
		var p: Vector2 = pickets[i][0]
		var dir: Vector2 = pickets[i][1]
		var h := rng.randf_range(0.9, 1.1)
		var xf := Transform3D(Basis(Vector3.UP, -atan2(dir.y, dir.x) + rng.randf_range(-0.04, 0.04)), to3(p, h * 0.5))
		xf.basis = xf.basis.scaled(Vector3(1.0, h, 1.0))
		mm.set_instance_transform(i, xf)
	var mmi := MultiMeshInstance3D.new()
	mmi.multimesh = mm
	mmi.material_override = wood
	add_child(mmi)


# --- Gardens and woodpiles -------------------------------------------------

func _build_garden(plot: Dictionary) -> Dictionary:
	var g: Rect2 = plot.garden
	var soil := StandardMaterial3D.new()
	soil.albedo_color = Color8(66, 50, 34)
	soil.roughness = 1.0
	var bed := MeshInstance3D.new()
	var box := BoxMesh.new()
	box.size = Vector3(g.size.x, 0.12, g.size.y)
	bed.mesh = box
	bed.material_override = soil
	bed.position = rect_center(g, 0.06)
	add_child(bed)

	# A plant every half metre along rows 1.5 m apart.
	var spots := []
	var y := g.position.y + 1.0
	while y < g.end.y - 0.5:
		var x := g.position.x + 0.5
		while x < g.end.x - 0.4:
			spots.append(Vector2(x, y))
			x += 0.5
		y += 1.5
	var plant := SphereMesh.new()
	plant.radius = 0.24
	plant.height = 0.32
	var mm := MultiMesh.new()
	mm.transform_format = MultiMesh.TRANSFORM_3D
	mm.mesh = plant
	mm.instance_count = spots.size()
	var rng := RandomNumberGenerator.new()
	rng.seed = hash(plot.id) + 7
	for i in spots.size():
		var s := rng.randf_range(0.8, 1.2)
		mm.set_instance_transform(i, Transform3D(Basis.from_scale(Vector3(s, s, s)), to3(spots[i], 0.2)))
	var crop := StandardMaterial3D.new()
	crop.roughness = 0.8
	var mmi := MultiMeshInstance3D.new()
	mmi.multimesh = mm
	mmi.material_override = crop
	add_child(mmi)
	return {"multimesh": mm, "instance": mmi, "material": crop, "total": spots.size()}


func _build_woodpile(plot: Dictionary, wood: Material) -> Dictionary:
	var w: Rect2 = plot.woodpile
	var log := CylinderMesh.new()
	log.top_radius = 0.13
	log.bottom_radius = 0.13
	log.height = w.size.y - 0.2
	log.radial_segments = 8
	var spots := []
	var per_row := int(w.size.x / 0.27)
	for row in 6:
		for i in per_row - (row % 2):
			var x := w.position.x + 0.14 + i * 0.27 + (row % 2) * 0.135
			spots.append(Vector3(x, 0.14 + row * 0.24, w.get_center().y))
	var mm := MultiMesh.new()
	mm.transform_format = MultiMesh.TRANSFORM_3D
	mm.mesh = log
	mm.instance_count = spots.size()
	for i in spots.size():
		mm.set_instance_transform(i, Transform3D(Basis(Vector3.RIGHT, PI / 2), spots[i]))
	var mmi := MultiMeshInstance3D.new()
	mmi.multimesh = mm
	mmi.material_override = wood
	add_child(mmi)
	return {"multimesh": mm, "capacity": spots.size()}


## Gardens grow and get harvested; woodpiles shrink as firewood is burned.
func update_households(households: Array, time: Dictionary) -> void:
	var snow: float = time.snow
	for i in households.size():
		var h: Dictionary = households[i]
		var garden: Dictionary = h.garden
		var g: Dictionary = _gardens[i]
		var standing := 1.0
		if garden.growth >= 1.0:
			standing = 0.0 if garden.expected <= 0.0 else garden.in_ground / garden.expected
		var shown := int(g.total * garden.planted * standing)
		(g.multimesh as MultiMesh).visible_instance_count = shown if snow < 0.6 else 0
		var young := Color8(132, 176, 86)
		var grown := Color8(70, 120, 52)
		var ripe := Color8(150, 140, 66)
		g.material.albedo_color = ripe if garden.growth >= 1.0 else young.lerp(grown, garden.growth)
		var s: float = 0.35 + 0.65 * clampf(garden.growth, 0.1, 1.0)
		(g.instance as Node3D).scale = Vector3(1, s, 1)

		var wp: Dictionary = _woodpiles[i]
		var fill := clampf(h.firewood / 150.0, 0.0, 1.0)
		(wp.multimesh as MultiMesh).visible_instance_count = int(ceil(wp.capacity * fill))


# --- Pump, railway, forest, snow ------------------------------------------

func _build_pump(street: Dictionary, wood: Material) -> void:
	var p: Vector2 = street.pump
	var stone := StandardMaterial3D.new()
	stone.albedo_color = Color8(120, 116, 108)
	var iron := StandardMaterial3D.new()
	iron.albedo_color = Color8(40, 56, 50)
	iron.metallic = 0.6
	iron.roughness = 0.5
	var base := MeshInstance3D.new()
	base.mesh = BoxMesh.new()
	(base.mesh as BoxMesh).size = Vector3(2.2, 0.3, 2.2)
	base.material_override = stone
	base.position = to3(p, 0.15)
	add_child(base)
	var post := MeshInstance3D.new()
	post.mesh = CylinderMesh.new()
	(post.mesh as CylinderMesh).top_radius = 0.12
	(post.mesh as CylinderMesh).bottom_radius = 0.15
	(post.mesh as CylinderMesh).height = 1.3
	post.material_override = iron
	post.position = to3(p, 0.95)
	add_child(post)
	var handle := MeshInstance3D.new()
	handle.mesh = BoxMesh.new()
	(handle.mesh as BoxMesh).size = Vector3(0.9, 0.05, 0.05)
	handle.material_override = iron
	handle.position = to3(p + Vector2(0.4, 0), 1.45)
	handle.rotation.z = 0.35
	add_child(handle)
	var trough := MeshInstance3D.new()
	trough.mesh = BoxMesh.new()
	(trough.mesh as BoxMesh).size = Vector3(0.9, 0.35, 0.5)
	trough.material_override = wood
	trough.position = to3(p + Vector2(0, 0.6), 0.45)
	add_child(trough)


func _build_railway(street: Dictionary) -> void:
	var y: float = street.railway_y
	var world_rect: Rect2 = street.world
	var steel := StandardMaterial3D.new()
	steel.albedo_color = Color8(80, 80, 86)
	steel.metallic = 0.7
	steel.roughness = 0.4
	var sleeper_mat := StandardMaterial3D.new()
	sleeper_mat.albedo_color = Color8(70, 54, 40)
	var x0 := world_rect.position.x - 200.0
	var length := world_rect.size.x + 400.0
	for offset in [-0.72, 0.72]:
		var rail := MeshInstance3D.new()
		rail.mesh = BoxMesh.new()
		(rail.mesh as BoxMesh).size = Vector3(length, 0.12, 0.07)
		rail.material_override = steel
		rail.position = Vector3(x0 + length * 0.5, 0.2, y + offset)
		add_child(rail)
	var mm := MultiMesh.new()
	mm.transform_format = MultiMesh.TRANSFORM_3D
	mm.mesh = BoxMesh.new()
	(mm.mesh as BoxMesh).size = Vector3(0.25, 0.12, 2.4)
	var n := int(length / 0.7)
	mm.instance_count = n
	for i in n:
		mm.set_instance_transform(i, Transform3D(Basis(), Vector3(x0 + i * 0.7, 0.08, y)))
	var mmi := MultiMeshInstance3D.new()
	mmi.multimesh = mm
	mmi.material_override = sleeper_mat
	add_child(mmi)


func _build_forest(street: Dictionary) -> void:
	var world_rect: Rect2 = street.world
	var area: Rect2 = street.plots[0].bounds
	for p in street.plots:
		area = area.merge(p.bounds)
	var clearing := area.grow(8.0)
	var road: Rect2 = (street.road as Rect2).grow(2.5)
	var railway_y: float = street.railway_y
	var rng := RandomNumberGenerator.new()
	rng.seed = 1900

	var pines := []
	var birches := []
	var bounds := world_rect.grow(40.0)
	for i in 4200:
		var p := Vector2(rng.randf_range(bounds.position.x, bounds.end.x), rng.randf_range(bounds.position.y, bounds.end.y))
		if clearing.has_point(p) or (p.y > road.position.y and p.y < road.end.y) or absf(p.y - railway_y) < 5.0:
			continue
		var size := rng.randf_range(0.75, 1.35)
		if rng.randf() < 0.3:
			birches.append([p, size])
		else:
			pines.append([p, size])

	_pine_material = StandardMaterial3D.new()
	_pine_material.albedo_color = PINE
	_pine_material.roughness = 0.95
	_birch_material = StandardMaterial3D.new()
	_birch_material.roughness = 0.9
	var bark := StandardMaterial3D.new()
	bark.albedo_color = Color8(60, 46, 34)
	var birch_bark := StandardMaterial3D.new()
	birch_bark.albedo_color = Color8(214, 210, 200)

	# Pines: a trunk and three stacked cones.
	var cone := CylinderMesh.new()
	cone.top_radius = 0.0
	cone.bottom_radius = 2.2
	cone.height = 4.5
	cone.radial_segments = 8
	var trunk := CylinderMesh.new()
	trunk.top_radius = 0.18
	trunk.bottom_radius = 0.28
	trunk.height = 3.0
	trunk.radial_segments = 6
	_scatter(trunk, bark, pines, func(p, s): return [Vector3(p.x, 1.5 * s, p.y)])
	var pine_layers := func(p, s): return [Vector3(p.x, 4.0 * s, p.y), Vector3(p.x, 6.5 * s, p.y), Vector3(p.x, 8.8 * s, p.y)]
	_scatter(cone, _pine_material, pines, pine_layers, [1.0, 0.8, 0.55])

	# Birches: a pale trunk and a rounded crown.
	var birch_trunk := CylinderMesh.new()
	birch_trunk.top_radius = 0.1
	birch_trunk.bottom_radius = 0.16
	birch_trunk.height = 7.0
	birch_trunk.radial_segments = 6
	var crown := SphereMesh.new()
	crown.radius = 2.0
	crown.height = 4.6
	crown.radial_segments = 10
	crown.rings = 6
	_scatter(birch_trunk, birch_bark, birches, func(p, s): return [Vector3(p.x, 3.5 * s, p.y)])
	_scatter(crown, _birch_material, birches, func(p, s): return [Vector3(p.x, 7.0 * s, p.y)])


## Places one mesh at every tree. `where` returns the positions for a tree;
## `shrink` optionally scales each of those positions' copies down.
func _scatter(mesh: Mesh, material: Material, trees: Array, where: Callable, shrink := []) -> void:
	var transforms := []
	for t in trees:
		var p: Vector2 = t[0]
		var s: float = t[1]
		var spots: Array = where.call(p, s)
		for i in spots.size():
			var k: float = s * (shrink[i] if i < shrink.size() else 1.0)
			transforms.append(Transform3D(Basis.from_scale(Vector3(k, s, k)), spots[i]))
	var mm := MultiMesh.new()
	mm.transform_format = MultiMesh.TRANSFORM_3D
	mm.mesh = mesh
	mm.instance_count = transforms.size()
	for i in transforms.size():
		mm.set_instance_transform(i, transforms[i])
	var mmi := MultiMeshInstance3D.new()
	mmi.multimesh = mm
	mmi.material_override = material
	add_child(mmi)


func _build_snowfall() -> void:
	snowfall = GPUParticles3D.new()
	snowfall.amount = 6000
	snowfall.lifetime = 9.0
	snowfall.preprocess = 9.0
	snowfall.visibility_aabb = AABB(Vector3(-80, -40, -80), Vector3(160, 80, 160))
	var process := ParticleProcessMaterial.new()
	process.emission_shape = ParticleProcessMaterial.EMISSION_SHAPE_BOX
	process.emission_box_extents = Vector3(70, 1, 70)
	process.direction = Vector3(0.2, -1, 0.1)
	process.spread = 15.0
	process.initial_velocity_min = 2.0
	process.initial_velocity_max = 3.5
	process.gravity = Vector3(0, -0.5, 0)
	process.turbulence_enabled = true
	process.turbulence_noise_strength = 0.6
	snowfall.process_material = process
	var flake := QuadMesh.new()
	flake.size = Vector2(0.07, 0.07)
	var flake_mat := StandardMaterial3D.new()
	flake_mat.albedo_color = Color(1, 1, 1, 0.9)
	flake_mat.transparency = BaseMaterial3D.TRANSPARENCY_ALPHA
	flake_mat.shading_mode = BaseMaterial3D.SHADING_MODE_UNSHADED
	flake_mat.billboard_mode = BaseMaterial3D.BILLBOARD_ENABLED
	flake.material = flake_mat
	snowfall.draw_pass_1 = flake
	snowfall.emitting = false
	add_child(snowfall)


## Keeps the snowfall over wherever the camera is looking.
func follow_view(target: Vector3) -> void:
	snowfall.global_position = target + Vector3(0, 28, 0)
