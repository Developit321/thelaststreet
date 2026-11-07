extends Node3D
## The street's surroundings: ground, road, grass, fences, gardens,
## woodpiles, sheds and clutter, the pump, the railway and the forest, plus
## the sun, sky, fog and snowfall. Visual only: everything it shows comes
## from the simulation.
##
## Street coordinates are metres with x east and y south; in Godot that's
## x and z, with y up.

const SEASONS := 4

# Colours at the middle of each season (spring, summer, autumn, winter before
# snow). Between them the world blends smoothly.
const PALETTE := {
	"grass_a": [Color8(86, 104, 52), Color8(78, 102, 46), Color8(112, 100, 54), Color8(104, 98, 76)],
	"grass_b": [Color8(110, 112, 62), Color8(126, 120, 64), Color8(140, 116, 64), Color8(120, 110, 86)],
	"forest_a": [Color8(46, 56, 34), Color8(42, 56, 32), Color8(72, 58, 36), Color8(60, 58, 50)],
	"forest_b": [Color8(64, 60, 40), Color8(60, 62, 38), Color8(96, 72, 42), Color8(76, 70, 60)],
	"birch": [Color8(108, 132, 74), Color8(74, 102, 48), Color8(176, 128, 46), Color8(122, 110, 98)],
	"tuft": [Color8(118, 140, 70), Color8(110, 136, 60), Color8(156, 136, 74), Color8(140, 128, 100)],
}
const WET := [0.8, 0.45, 0.7, 0.3]  # how muddy the ground is in each season
const PINE := Color8(30, 50, 32)
const DIRT := Color8(78, 60, 42)

var sun: DirectionalLight3D
var moon: DirectionalLight3D
var environment: Environment
var sky_material: ProceduralSkyMaterial
var snowfall: GPUParticles3D

var _shaders := []        # ground-type ShaderMaterials
var _road: ShaderMaterial
var _gardens := {}        # plot id -> {multimesh, instance, material, total}
var _woodpiles := {}      # plot id -> {multimesh, capacity}
var _birch_material: StandardMaterial3D
var _birch_crowns: MultiMeshInstance3D
var _pine_material: StandardMaterial3D
var _tuft_material: StandardMaterial3D
var _tufts: MultiMesh
var _noise_big: NoiseTexture2D
var _noise_small: NoiseTexture2D


static func to3(p: Vector2, y := 0.0) -> Vector3:
	return Vector3(p.x, y, p.y)


static func rect_center(r: Rect2, y := 0.0) -> Vector3:
	return Vector3(r.position.x + r.size.x * 0.5, y, r.position.y + r.size.y * 0.5)


func build(street: Dictionary, wood_material: Material) -> void:
	_noise_big = _noise(11, 0.01, 512)
	_noise_small = _noise(23, 0.03, 512)
	_build_light_and_sky()
	_build_ground(street)
	_build_railway(street)
	_build_pump(street, wood_material)
	for plot in street.plots:
		_build_fence(plot, wood_material)
		_gardens[plot.id] = _build_garden(plot)
		_woodpiles[plot.id] = _build_woodpile(plot, wood_material)
		_build_yard_clutter(plot, wood_material)
	_build_cart(street, wood_material)
	_build_grass(street)
	_build_forest(street)
	_build_snowfall()


func _noise(seed_value: int, frequency: float, size: int) -> NoiseTexture2D:
	var n := FastNoiseLite.new()
	n.seed = seed_value
	n.frequency = frequency
	n.fractal_octaves = 5
	var tex := NoiseTexture2D.new()
	tex.noise = n
	tex.seamless = true
	tex.width = size
	tex.height = size
	tex.generate_mipmaps = true
	return tex


# --- Light, sky, fog -------------------------------------------------------

func _build_light_and_sky() -> void:
	sky_material = ProceduralSkyMaterial.new()
	sky_material.sky_curve = 0.1
	sky_material.ground_bottom_color = Color(0.16, 0.17, 0.13)
	sky_material.sun_angle_max = 20.0
	var sky := Sky.new()
	sky.sky_material = sky_material

	environment = Environment.new()
	environment.background_mode = Environment.BG_SKY
	environment.sky = sky
	environment.ambient_light_source = Environment.AMBIENT_SOURCE_SKY
	environment.ambient_light_sky_contribution = 0.6
	environment.tonemap_mode = Environment.TONE_MAPPER_AGX
	environment.tonemap_exposure = 1.0
	environment.ssao_enabled = true
	environment.ssao_radius = 1.5
	environment.ssao_intensity = 2.4
	environment.ssil_enabled = true
	environment.ssil_intensity = 0.8
	environment.glow_enabled = true
	environment.glow_intensity = 0.35
	environment.glow_bloom = 0.05
	environment.fog_enabled = true
	environment.fog_density = 0.0025
	environment.fog_aerial_perspective = 0.7
	environment.fog_sky_affect = 0.4
	environment.adjustment_enabled = true
	environment.adjustment_saturation = 1.08
	environment.adjustment_contrast = 1.06
	var world_env := WorldEnvironment.new()
	world_env.environment = environment
	add_child(world_env)

	sun = DirectionalLight3D.new()
	sun.shadow_enabled = true
	sun.shadow_blur = 1.2
	sun.directional_shadow_max_distance = 220.0
	sun.directional_shadow_split_1 = 0.08
	sun.directional_shadow_split_2 = 0.22
	sun.directional_shadow_split_3 = 0.5
	sun.light_angular_distance = 1.0
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
	# Golden when the sun is low, white at midday.
	var low := 1.0 - clampf(elevation / deg_to_rad(28.0), 0.0, 1.0)
	sun.light_color = Color(1.0, 0.94, 0.85).lerp(Color(1.0, 0.62, 0.34), low)
	sun.light_energy = (2.1 + low * 0.6) * light
	sun.visible = light > 0.01
	moon.light_energy = 0.24 * (1.0 - light)
	moon.visible = light < 0.99

	var day_top := Color(0.3, 0.46, 0.72)
	var day_horizon := Color(0.7, 0.75, 0.78).lerp(Color(0.98, 0.7, 0.46), low * 0.85)
	var night_top := Color(0.01, 0.02, 0.05)
	var night_horizon := Color(0.04, 0.06, 0.11)
	var overcast: float = 0.5 if time.season == 3 else 0.0
	day_top = day_top.lerp(Color(0.62, 0.65, 0.68), overcast)
	sky_material.sky_top_color = night_top.lerp(day_top, light)
	sky_material.sky_horizon_color = night_horizon.lerp(day_horizon, light)
	sky_material.ground_horizon_color = sky_material.sky_horizon_color
	environment.ambient_light_energy = lerpf(0.14, 0.55, light)

	var snowing: bool = time.snowing
	environment.fog_density = 0.0022 + overcast * 0.004 + (0.012 if snowing else 0.0)
	environment.fog_light_color = sky_material.sky_horizon_color.lerp(Color(1, 1, 1), 0.2)
	environment.fog_sun_scatter = 0.25 * low
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


func _ground(dirt_amount: float) -> ShaderMaterial:
	var m := ShaderMaterial.new()
	m.shader = load("res://godot/shaders/ground.gdshader")
	m.set_shader_parameter("noise_big", _noise_big)
	m.set_shader_parameter("noise_small", _noise_small)
	m.set_shader_parameter("dirt", DIRT)
	m.set_shader_parameter("dirt_amount", dirt_amount)
	_shaders.append(m)
	return m


func _build_ground(street: Dictionary) -> void:
	var world_rect: Rect2 = street.world
	var forest := _ground(0.12)
	forest.set_meta("forest", true)
	_flat("Forest floor", world_rect.grow(260.0), -0.03, forest)

	var area: Rect2 = street.plots[0].bounds
	for p in street.plots:
		area = area.merge(p.bounds)
	_flat("Clearing", area.grow(10.0), 0.0, _ground(0.18))
	for p in street.plots:
		_flat("Yard", p.bounds, 0.01, _ground(0.3))

	var road: Rect2 = street.road
	_road = ShaderMaterial.new()
	_road.shader = load("res://godot/shaders/road.gdshader")
	_road.set_shader_parameter("noise_big", _noise_big)
	_road.set_shader_parameter("noise_small", _noise_small)
	_road.set_shader_parameter("road_center", road.get_center().y)
	_road.set_shader_parameter("road_half", road.size.y * 0.5 + 0.6)
	_flat("Road", Rect2(road.position.x - 260.0, road.position.y - 0.6, road.size.x + 520.0, road.size.y + 1.2), 0.02, _road)


## Season and snow colours for the ground, trees, grass and gardens.
func update_season(time: Dictionary) -> void:
	var pos: float = time.season + time.season_progress - 0.5
	if pos < 0.0:
		pos += SEASONS
	var from := int(floorf(pos)) % SEASONS
	var to := (from + 1) % SEASONS
	var t := pos - floorf(pos)
	var snow: float = time.snow
	var wet: float = lerpf(WET[from], WET[to], t)

	var pick := func(key: String) -> Color:
		return (PALETTE[key][from] as Color).lerp(PALETTE[key][to], t)
	for m in _shaders:
		var forest: bool = m.has_meta("forest")
		m.set_shader_parameter("grass_a", pick.call("forest_a" if forest else "grass_a"))
		m.set_shader_parameter("grass_b", pick.call("forest_b" if forest else "grass_b"))
		m.set_shader_parameter("snow", snow)
		m.set_shader_parameter("wet", wet)
	_road.set_shader_parameter("snow", snow)
	_road.set_shader_parameter("wet", wet)
	_birch_material.albedo_color = pick.call("birch")
	var leafless: bool = time.season == 3 or (time.season == 2 and time.season_progress > 0.85) or (time.season == 0 and time.season_progress < 0.15)
	_birch_crowns.visible = not leafless
	_pine_material.albedo_color = Color(1, 1, 1).lerp(Color(1.8, 1.9, 2.0), snow * 0.3)
	_tuft_material.albedo_color = pick.call("tuft")
	_tufts.visible_instance_count = -1 if snow < 0.45 else 0


# --- Fences, gardens, woodpiles, clutter -------------------------------------

func _build_fence(plot: Dictionary, wood: Material) -> void:
	var b: Rect2 = plot.bounds
	var gate_x: float = (plot.house as Rect2).get_center().x
	var front_y := b.end.y if plot.north else b.position.y
	var back_y := b.position.y if plot.north else b.end.y
	var sides := [
		[Vector2(b.position.x, back_y), Vector2(b.end.x, back_y)],
		[Vector2(b.position.x, b.position.y), Vector2(b.position.x, b.end.y)],
		[Vector2(b.end.x, b.position.y), Vector2(b.end.x, b.end.y)],
		[Vector2(b.position.x, front_y), Vector2(gate_x - 1.5, front_y)],
		[Vector2(gate_x + 1.5, front_y), Vector2(b.end.x, front_y)],
	]
	var rng := RandomNumberGenerator.new()
	rng.seed = hash(plot.id)
	var pickets := []
	for s in sides:
		var a: Vector2 = s[0]
		var c: Vector2 = s[1]
		var length := a.distance_to(c)
		var dir := (c - a).normalized()
		var n := int(length / 0.14)
		for i in n:
			pickets.append([a + dir * (i + 0.5) * 0.14, dir])
		for h in [0.3, 0.8]:
			var rail := MeshInstance3D.new()
			var box := BoxMesh.new()
			box.size = Vector3(length, 0.06, 0.05)
			rail.mesh = box
			rail.material_override = wood
			rail.position = to3((a + c) * 0.5, h)
			rail.rotation.y = -atan2(dir.y, dir.x)
			add_child(rail)
		# Posts every couple of metres.
		var posts := int(length / 2.2) + 1
		for i in posts:
			var post := MeshInstance3D.new()
			var pm := BoxMesh.new()
			pm.size = Vector3(0.12, 1.25, 0.12)
			post.mesh = pm
			post.material_override = wood
			post.position = to3(a + dir * (length * i / max(1, posts - 1)), 0.62)
			add_child(post)

	var mm := MultiMesh.new()
	mm.transform_format = MultiMesh.TRANSFORM_3D
	var picket := BoxMesh.new()
	picket.size = Vector3(0.1, 1.0, 0.025)
	mm.mesh = picket
	mm.instance_count = pickets.size()
	for i in pickets.size():
		var p: Vector2 = pickets[i][0]
		var dir: Vector2 = pickets[i][1]
		var h := rng.randf_range(0.92, 1.08)
		var lean := Basis(Vector3(dir.x, 0, dir.y), rng.randf_range(-0.05, 0.05))
		var xf := Transform3D(lean * Basis(Vector3.UP, -atan2(dir.y, dir.x)), to3(p, h * 0.5))
		xf.basis = xf.basis.scaled(Vector3(1.0, h, 1.0))
		mm.set_instance_transform(i, xf)
	var mmi := MultiMeshInstance3D.new()
	mmi.multimesh = mm
	mmi.material_override = wood
	add_child(mmi)


func _build_garden(plot: Dictionary) -> Dictionary:
	var g: Rect2 = plot.garden
	var soil := StandardMaterial3D.new()
	soil.albedo_color = Color8(58, 44, 30)
	soil.roughness = 1.0
	var bed := MeshInstance3D.new()
	var box := BoxMesh.new()
	box.size = Vector3(g.size.x, 0.1, g.size.y)
	bed.mesh = box
	bed.material_override = soil
	bed.position = rect_center(g, 0.05)
	add_child(bed)

	# Cabbages and potato plants: one every 0.45 m along rows 0.9 m apart.
	var spots := []
	var y := g.position.y + 0.6
	while y < g.end.y - 0.4:
		var x := g.position.x + 0.4
		while x < g.end.x - 0.3:
			spots.append(Vector2(x, y))
			x += 0.45
		y += 0.9
	var plant := SphereMesh.new()
	plant.radius = 0.22
	plant.height = 0.3
	plant.radial_segments = 8
	plant.rings = 4
	var mm := MultiMesh.new()
	mm.transform_format = MultiMesh.TRANSFORM_3D
	mm.use_colors = true
	mm.mesh = plant
	mm.instance_count = spots.size()
	var rng := RandomNumberGenerator.new()
	rng.seed = hash(plot.id) + 7
	for i in spots.size():
		var s := rng.randf_range(0.75, 1.25)
		mm.set_instance_transform(i, Transform3D(Basis.from_scale(Vector3(s, s * rng.randf_range(0.8, 1.2), s)), to3(spots[i] + Vector2(rng.randf_range(-0.06, 0.06), 0), 0.17)))
		mm.set_instance_color(i, Color(1, 1, 1).darkened(rng.randf_range(0.0, 0.3)))
	var crop := StandardMaterial3D.new()
	crop.roughness = 0.7
	crop.vertex_color_use_as_albedo = true
	crop.vertex_color_is_srgb = true
	var mmi := MultiMeshInstance3D.new()
	mmi.multimesh = mm
	mmi.material_override = crop
	add_child(mmi)
	return {"multimesh": mm, "instance": mmi, "material": crop, "soil": soil, "total": spots.size()}


func _build_woodpile(plot: Dictionary, wood: Material) -> Dictionary:
	var w: Rect2 = plot.woodpile
	var log := CylinderMesh.new()
	log.top_radius = 0.12
	log.bottom_radius = 0.12
	log.height = w.size.y - 0.3
	log.radial_segments = 8
	var spots := []
	var per_row := int(w.size.x / 0.25)
	for row in 7:
		for i in per_row - (row % 2):
			var x := w.position.x + 0.13 + i * 0.25 + (row % 2) * 0.125
			spots.append(Vector3(x, 0.13 + row * 0.22, w.get_center().y))
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
	# A chopping block with the axe left in it.
	var block := MeshInstance3D.new()
	var bm := CylinderMesh.new()
	bm.top_radius = 0.3
	bm.bottom_radius = 0.34
	bm.height = 0.5
	block.mesh = bm
	block.material_override = wood
	block.position = Vector3(w.position.x - 1.0, 0.25, w.get_center().y + 1.6)
	add_child(block)
	return {"multimesh": mm, "capacity": spots.size()}


func _shed(at: Vector2, size: Vector2, facing_south: bool, wood: Material) -> void:
	var body := MeshInstance3D.new()
	var bm := BoxMesh.new()
	bm.size = Vector3(size.x, 2.0, size.y)
	body.mesh = bm
	body.material_override = wood
	body.position = to3(at, 1.0)
	add_child(body)
	var roof := MeshInstance3D.new()
	var rm := BoxMesh.new()
	rm.size = Vector3(size.x + 0.5, 0.1, size.y + 0.6)
	roof.mesh = rm
	roof.material_override = wood
	roof.position = to3(at, 2.2)
	roof.rotation.x = 0.2 if facing_south else -0.2
	add_child(roof)


func _build_yard_clutter(plot: Dictionary, wood: Material) -> void:
	var b: Rect2 = plot.bounds
	var wp: Rect2 = plot.woodpile
	var house: Rect2 = plot.house
	var rng := RandomNumberGenerator.new()
	rng.seed = hash(plot.id) + 31
	# A shed beside the woodpile.
	var shed_y := wp.end.y + 2.6 if plot.north else wp.position.y - 2.6
	_shed(Vector2(wp.get_center().x, shed_y), Vector2(3.2, 2.4), plot.north, wood)
	# Barrels and a bench by the house.
	var barrel := CylinderMesh.new()
	barrel.top_radius = 0.28
	barrel.bottom_radius = 0.3
	barrel.height = 0.85
	barrel.radial_segments = 12
	var side := 1.0 if rng.randf() < 0.5 else -1.0
	var bx := house.get_center().x + side * (house.size.x * 0.5 + 0.6)
	for i in rng.randi_range(1, 3):
		var mi := MeshInstance3D.new()
		mi.mesh = barrel
		mi.material_override = wood
		mi.position = Vector3(bx, 0.43, house.get_center().y + i * 0.7 - 0.7)
		add_child(mi)
	var bench := MeshInstance3D.new()
	var bench_mesh := BoxMesh.new()
	bench_mesh.size = Vector3(1.6, 0.08, 0.4)
	bench.mesh = bench_mesh
	bench.material_override = wood
	var front_y := house.end.y + 0.6 if plot.north else house.position.y - 0.6
	bench.position = Vector3(house.position.x + 1.6, 0.45, front_y)
	add_child(bench)


func _build_cart(street: Dictionary, wood: Material) -> void:
	var road: Rect2 = street.road
	var at := Vector3(street.pump.x - 9.0, 0, road.get_center().y - 1.6)
	var bed := MeshInstance3D.new()
	var bm := BoxMesh.new()
	bm.size = Vector3(2.6, 0.5, 1.3)
	bed.mesh = bm
	bed.material_override = wood
	bed.position = at + Vector3(0, 0.95, 0)
	add_child(bed)
	var wheel := CylinderMesh.new()
	wheel.top_radius = 0.55
	wheel.bottom_radius = 0.55
	wheel.height = 0.08
	for dx in [-0.9, 0.9]:
		for dz in [-0.72, 0.72]:
			var w := MeshInstance3D.new()
			w.mesh = wheel
			w.material_override = wood
			w.position = at + Vector3(dx, 0.55, dz)
			w.rotation.x = PI / 2
			add_child(w)
	for dz in [-0.35, 0.35]:
		var shaft := MeshInstance3D.new()
		var sm := BoxMesh.new()
		sm.size = Vector3(2.4, 0.08, 0.08)
		shaft.mesh = sm
		shaft.material_override = wood
		shaft.position = at + Vector3(2.4, 0.7, dz)
		shaft.rotation.z = -0.25
		add_child(shaft)


## Gardens grow and get harvested; woodpiles shrink as firewood is burned.
## Plots with nobody living there yet follow the seasons on their own.
func update_households(households: Array, time: Dictionary) -> void:
	var by_plot := {}
	for h in households:
		by_plot[h.plot] = h
	var snow: float = time.snow
	var season: int = time.season
	var progress: float = time.season_progress
	for plot_id in _gardens:
		var g: Dictionary = _gardens[plot_id]
		var planted := 0.0
		var growth := 0.0
		var standing := 1.0
		var firewood := 90.0
		if by_plot.has(plot_id):
			var h: Dictionary = by_plot[plot_id]
			var garden: Dictionary = h.garden
			planted = garden.planted
			growth = garden.growth
			if growth >= 1.0:
				standing = 0.0 if garden.expected <= 0.0 else garden.in_ground / garden.expected
			firewood = h.firewood
		else:
			match season:
				0: planted = clampf(progress * 1.5, 0.0, 1.0); growth = progress * 0.3
				1: planted = 1.0; growth = 0.3 + progress * 0.7
				2: planted = 1.0; growth = 1.0; standing = 1.0 - progress
				_: planted = 0.0
		(g.multimesh as MultiMesh).visible_instance_count = int(g.total * planted * standing) if snow < 0.6 else 0
		var young := Color8(108, 146, 70)
		var grown := Color8(46, 86, 36)
		var ripe := Color8(98, 112, 50)
		g.material.albedo_color = ripe if growth >= 1.0 else young.lerp(grown, growth)
		(g.instance as Node3D).scale = Vector3(1, 0.4 + 0.6 * clampf(growth, 0.1, 1.0), 1)
		(g.soil as StandardMaterial3D).albedo_color = Color8(58, 44, 30).lerp(Color(0.88, 0.9, 0.93), clampf(snow * 1.3, 0.0, 1.0))

		var wp: Dictionary = _woodpiles[plot_id]
		(wp.multimesh as MultiMesh).visible_instance_count = int(ceil(wp.capacity * clampf(firewood / 150.0, 0.0, 1.0)))


# --- Grass ---------------------------------------------------------------------

## A tuft of grass: five thin, slightly curved blades.
func _tuft_mesh() -> ArrayMesh:
	var st := SurfaceTool.new()
	st.begin(Mesh.PRIMITIVE_TRIANGLES)
	var rng := RandomNumberGenerator.new()
	rng.seed = 5
	for b in 5:
		var angle := rng.randf_range(0, TAU)
		var dir := Vector3(cos(angle), 0, sin(angle))
		var side := Vector3(-dir.z, 0, dir.x) * 0.02
		var lean := dir * rng.randf_range(0.05, 0.16)
		var h := rng.randf_range(0.22, 0.42)
		var base := dir * 0.03
		st.set_color(Color(0.55, 0.55, 0.5))
		st.add_vertex(base - side)
		st.add_vertex(base + side)
		st.set_color(Color(1, 1, 1))
		st.add_vertex(base + lean + Vector3(0, h, 0))
	st.generate_normals()
	return st.commit()


func _build_grass(street: Dictionary) -> void:
	var area: Rect2 = street.plots[0].bounds
	for p in street.plots:
		area = area.merge(p.bounds)
	area = area.grow(9.0)
	var road: Rect2 = (street.road as Rect2).grow(0.3)
	var blocked := []
	for p in street.plots:
		blocked.append((p.house as Rect2).grow(0.8))
		blocked.append((p.garden as Rect2).grow(0.2))
		blocked.append((p.woodpile as Rect2).grow(1.5))
	var rng := RandomNumberGenerator.new()
	rng.seed = 77
	var spots := []
	for i in 40000:
		var p := Vector2(rng.randf_range(area.position.x, area.end.x), rng.randf_range(area.position.y, area.end.y))
		if p.y > road.position.y and p.y < road.end.y:
			continue
		var hit := false
		for r in blocked:
			if r.has_point(p):
				hit = true
				break
		if hit:
			continue
		spots.append(p)
	_tufts = MultiMesh.new()
	_tufts.transform_format = MultiMesh.TRANSFORM_3D
	_tufts.use_colors = true
	_tufts.mesh = _tuft_mesh()
	_tufts.instance_count = spots.size()
	for i in spots.size():
		var s := rng.randf_range(0.7, 1.6)
		_tufts.set_instance_transform(i, Transform3D(Basis(Vector3.UP, rng.randf_range(0, TAU)).scaled(Vector3(s, s, s)), to3(spots[i], 0.0)))
		_tufts.set_instance_color(i, Color(1, 1, 1).darkened(rng.randf_range(0.0, 0.35)))
	_tuft_material = StandardMaterial3D.new()
	_tuft_material.vertex_color_use_as_albedo = true
	_tuft_material.vertex_color_is_srgb = true
	_tuft_material.cull_mode = BaseMaterial3D.CULL_DISABLED
	_tuft_material.roughness = 0.9
	var mmi := MultiMeshInstance3D.new()
	mmi.multimesh = _tufts
	mmi.material_override = _tuft_material
	mmi.cast_shadow = GeometryInstance3D.SHADOW_CASTING_SETTING_OFF
	add_child(mmi)


# --- Pump, railway, forest, snow ------------------------------------------

func _build_pump(street: Dictionary, wood: Material) -> void:
	var p: Vector2 = street.pump
	var stone := StandardMaterial3D.new()
	stone.albedo_color = Color8(110, 106, 98)
	var iron := StandardMaterial3D.new()
	iron.albedo_color = Color8(36, 50, 46)
	iron.metallic = 0.6
	iron.roughness = 0.5
	var base := MeshInstance3D.new()
	base.mesh = BoxMesh.new()
	(base.mesh as BoxMesh).size = Vector3(1.8, 0.25, 1.8)
	base.material_override = stone
	base.position = to3(p, 0.12)
	add_child(base)
	var post := MeshInstance3D.new()
	post.mesh = CylinderMesh.new()
	(post.mesh as CylinderMesh).top_radius = 0.1
	(post.mesh as CylinderMesh).bottom_radius = 0.14
	(post.mesh as CylinderMesh).height = 1.4
	post.material_override = iron
	post.position = to3(p, 0.95)
	add_child(post)
	var handle := MeshInstance3D.new()
	handle.mesh = BoxMesh.new()
	(handle.mesh as BoxMesh).size = Vector3(0.9, 0.05, 0.05)
	handle.material_override = iron
	handle.position = to3(p + Vector2(0.4, 0), 1.5)
	handle.rotation.z = 0.35
	add_child(handle)
	var trough := MeshInstance3D.new()
	trough.mesh = BoxMesh.new()
	(trough.mesh as BoxMesh).size = Vector3(0.9, 0.35, 0.5)
	trough.material_override = wood
	trough.position = to3(p + Vector2(0, 0.6), 0.42)
	add_child(trough)


func _build_railway(street: Dictionary) -> void:
	var y: float = street.railway_y
	var world_rect: Rect2 = street.world
	var steel := StandardMaterial3D.new()
	steel.albedo_color = Color8(80, 80, 86)
	steel.metallic = 0.7
	steel.roughness = 0.4
	var sleeper_mat := StandardMaterial3D.new()
	sleeper_mat.albedo_color = Color8(62, 48, 36)
	var ballast := StandardMaterial3D.new()
	ballast.albedo_color = Color8(92, 88, 82)
	var x0 := world_rect.position.x - 300.0
	var length := world_rect.size.x + 600.0
	var bed := MeshInstance3D.new()
	bed.mesh = BoxMesh.new()
	(bed.mesh as BoxMesh).size = Vector3(length, 0.3, 4.0)
	bed.material_override = ballast
	bed.position = Vector3(x0 + length * 0.5, 0.0, y)
	add_child(bed)
	for offset in [-0.72, 0.72]:
		var rail := MeshInstance3D.new()
		rail.mesh = BoxMesh.new()
		(rail.mesh as BoxMesh).size = Vector3(length, 0.12, 0.07)
		rail.material_override = steel
		rail.position = Vector3(x0 + length * 0.5, 0.3, y + offset)
		add_child(rail)
	var mm := MultiMesh.new()
	mm.transform_format = MultiMesh.TRANSFORM_3D
	mm.mesh = BoxMesh.new()
	(mm.mesh as BoxMesh).size = Vector3(0.25, 0.1, 2.4)
	var n := int(length / 0.7)
	mm.instance_count = n
	for i in n:
		mm.set_instance_transform(i, Transform3D(Basis(), Vector3(x0 + i * 0.7, 0.19, y)))
	var mmi := MultiMeshInstance3D.new()
	mmi.multimesh = mm
	mmi.material_override = sleeper_mat
	add_child(mmi)


func _build_forest(street: Dictionary) -> void:
	var world_rect: Rect2 = street.world
	var area: Rect2 = street.plots[0].bounds
	for p in street.plots:
		area = area.merge(p.bounds)
	var clearing := area.grow(11.0)
	var road: Rect2 = (street.road as Rect2).grow(3.0)
	var railway_y: float = street.railway_y
	var rng := RandomNumberGenerator.new()
	rng.seed = 1900

	var pines := []
	var birches := []
	var bounds := world_rect.grow(70.0)
	for i in 9000:
		var p := Vector2(rng.randf_range(bounds.position.x, bounds.end.x), rng.randf_range(bounds.position.y, bounds.end.y))
		if clearing.has_point(p) or (p.y > road.position.y and p.y < road.end.y) or absf(p.y - railway_y) < 6.0:
			continue
		# Birches like the sunny edge of the clearing; pines fill the forest.
		var edge := clearing.grow(14.0).has_point(p)
		var size := rng.randf_range(1.0, 1.9)
		if rng.randf() < (0.45 if edge else 0.15):
			birches.append([p, size * 0.9, rng.randf()])
		else:
			pines.append([p, size, rng.randf()])

	_pine_material = StandardMaterial3D.new()
	_pine_material.vertex_color_use_as_albedo = true
	_pine_material.vertex_color_is_srgb = true
	_pine_material.roughness = 0.95
	_birch_material = StandardMaterial3D.new()
	_birch_material.roughness = 0.9
	var bark := StandardMaterial3D.new()
	bark.albedo_color = Color8(56, 42, 30)
	var birch_bark := StandardMaterial3D.new()
	birch_bark.albedo_color = Color8(210, 206, 196)

	# Pines: a tall trunk and five narrowing tiers of branches.
	var cone := CylinderMesh.new()
	cone.top_radius = 0.0
	cone.bottom_radius = 1.6
	cone.height = 3.2
	cone.radial_segments = 9
	var trunk := CylinderMesh.new()
	trunk.top_radius = 0.12
	trunk.bottom_radius = 0.26
	trunk.height = 10.0
	trunk.radial_segments = 6
	_scatter(trunk, bark, pines, func(p, s): return [Vector3(p.x, 5.0 * s, p.y)])
	var tiers := []
	var shrink := []
	for k in 5:
		tiers.append(3.2 + k * 1.9)
		shrink.append(1.0 - k * 0.17)
	var pine_tiers := func(p, s): return tiers.map(func(h): return Vector3(p.x, h * s, p.y))
	_scatter(cone, _pine_material, pines, pine_tiers, shrink, true)

	# Birches: a tall pale trunk and a loose crown of three clumps.
	var birch_trunk := CylinderMesh.new()
	birch_trunk.top_radius = 0.07
	birch_trunk.bottom_radius = 0.14
	birch_trunk.height = 9.0
	birch_trunk.radial_segments = 6
	var crown := SphereMesh.new()
	crown.radius = 1.3
	crown.height = 2.4
	crown.radial_segments = 8
	crown.rings = 5
	_scatter(birch_trunk, birch_bark, birches, func(p, s): return [Vector3(p.x, 4.5 * s, p.y)])
	var clumps := func(p, s): return [Vector3(p.x, 7.2 * s, p.y), Vector3(p.x + 0.7 * s, 6.0 * s, p.y + 0.4 * s), Vector3(p.x - 0.6 * s, 5.6 * s, p.y - 0.5 * s)]
	_birch_crowns = _scatter(crown, _birch_material, birches, clumps, [1.0, 0.8, 0.75])


## Places one mesh at every tree. `where` returns the positions for a tree;
## `shrink` scales each of those copies' width; `tint` gives each tree a
## slightly different shade.
func _scatter(mesh: Mesh, material: Material, trees: Array, where: Callable, shrink := [], tint := false) -> MultiMeshInstance3D:
	var transforms := []
	var colours := []
	for t in trees:
		var p: Vector2 = t[0]
		var s: float = t[1]
		var spots: Array = where.call(p, s)
		var shade := Color8(20, 36, 22).lerp(Color8(34, 50, 28), t[2])
		for i in spots.size():
			var k: float = s * (shrink[i] if i < shrink.size() else 1.0)
			transforms.append(Transform3D(Basis(Vector3.UP, float(t[2]) * TAU).scaled(Vector3(k, s, k)), spots[i]))
			colours.append(shade)
	var mm := MultiMesh.new()
	mm.transform_format = MultiMesh.TRANSFORM_3D
	mm.use_colors = tint
	mm.mesh = mesh
	mm.instance_count = transforms.size()
	for i in transforms.size():
		mm.set_instance_transform(i, transforms[i])
		if tint:
			mm.set_instance_color(i, colours[i])
	var mmi := MultiMeshInstance3D.new()
	mmi.multimesh = mm
	mmi.material_override = material
	add_child(mmi)
	return mmi


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
