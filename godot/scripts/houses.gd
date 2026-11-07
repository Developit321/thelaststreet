extends Node3D
## The houses: the Blender model on every plot, with photo-textured
## materials, chimney smoke while the stove burns, windows that glow at
## night, and the cutaway that lifts the roof and lowers the walls so people
## inside can be seen.

const HOUSE_SCENE := "res://assets/models/lind_house.glb"
const TEXTURES := "res://assets/textures/"
const SURFACE := preload("res://godot/shaders/surface.gdshader")
const CHIMNEY := Vector3(-1.0, 7.05, 0.45)  # chimney top, relative to a north-side house

# Blender material name -> how to draw it in Godot.
# Photo textures: [texture, real size in metres, tint]. Plain: a colour.
const PHOTO := {
	"Logs (photo)": ["weathered_brown_planks", 2.4, Color(0.6, 0.58, 0.56)],
	"Trim (photo)": ["weathered_brown_planks", 2.0, Color(0.5, 0.48, 0.46)],
	"Shingles (photo)": ["roof_09", 3.2, Color(0.6, 0.58, 0.56)],
	"Shingles dark (photo)": ["roof_09", 3.0, Color(0.47, 0.46, 0.44)],
	"Shingles light (photo)": ["roof_09", 3.4, Color(0.7, 0.68, 0.65)],
	"Fieldstone (photo)": ["rustic_stone_wall", 1.3, Color(0.72, 0.71, 0.69)],
	"Chimney brick (photo)": ["red_brick_03", 1.0, Color(0.85, 0.85, 0.85)],
	"Red boards (photo)": ["distressed_painted_planks", 1.6, Color(0.62, 0.16, 0.12)],
	"Red door (photo)": ["distressed_painted_planks", 1.2, Color(0.66, 0.17, 0.12)],
	"Ochre paint (photo)": ["distressed_painted_planks", 1.0, Color(1.0, 0.68, 0.28)],
	"Floor (photo)": ["old_planks_02", 2.0, Color(0.8, 0.8, 0.8)],
	"Planks (photo)": ["old_planks_02", 1.6, Color(0.9, 0.9, 0.9)],
}
const PLAIN := {
	"Log end": Color8(54, 44, 32),
	"Moss": Color8(34, 44, 16),
	"Iron": Color8(16, 16, 16),
	"Stove tiles": Color8(214, 210, 198),
	"Wool red": Color8(110, 30, 22),
	"Wool blue": Color8(36, 52, 84),
	"Wool green": Color8(50, 70, 38),
	"Linen": Color8(204, 196, 178),
	"Pottery": Color8(136, 84, 52),
	"Lace": Color(0.78, 0.76, 0.7, 0.55),
	"Glass": Color(0.04, 0.05, 0.06, 0.75),
}
# Plainer houses swap the painted wood for bare weathered boards.
const PLAIN_SWAPS := {
	"Red boards (photo)": ["weathered_brown_planks", 1.6, Color(0.55, 0.52, 0.5)],
	"Red door (photo)": ["weathered_brown_planks", 1.2, Color(0.45, 0.42, 0.4)],
	"Ochre paint (photo)": ["weathered_brown_planks", 1.0, Color(0.62, 0.6, 0.57)],
}
# Small differences in tone between houses, so the street isn't a copy-paste.
const TONES := [1.0, 0.86, 1.1]
const ROOF_MATERIALS := ["Shingles (photo)", "Shingles dark (photo)", "Shingles light (photo)"]

var houses := []  # one per plot: {root, parts, glass, smoke, plot, open}
var _by_plot := {}
var _cache := {}
var _snowy := []  # every photo material, so snow can settle on them


func _photo(spec: Array, tone: float) -> ShaderMaterial:
	var id: String = spec[0]
	var m := ShaderMaterial.new()
	m.shader = SURFACE
	m.set_shader_parameter("albedo_tex", load(TEXTURES + id + "/" + id + "_diff_2k.jpg"))
	m.set_shader_parameter("normal_tex", load(TEXTURES + id + "/" + id + "_nor_gl_2k.jpg"))
	m.set_shader_parameter("rough_tex", load(TEXTURES + id + "/" + id + "_rough_2k.jpg"))
	m.set_shader_parameter("scale", 1.0 / float(spec[1]))
	var c: Color = spec[2]
	m.set_shader_parameter("tint", Color(c.r * tone, c.g * tone, c.b * tone))
	_snowy.append(m)
	return m


## The material for a Blender material name, for a painted or plain house of
## a given tone.
func material_for(name: String, painted := true, tone := 0) -> Material:
	var key := "%s|%s|%d" % [name, painted, tone]
	if _cache.has(key):
		return _cache[key]
	var m: Material
	if not painted and PLAIN_SWAPS.has(name):
		m = _photo(PLAIN_SWAPS[name], TONES[tone])
	elif PHOTO.has(name):
		var weathered: bool = name in ["Logs (photo)", "Trim (photo)"] or name in ROOF_MATERIALS
		m = _photo(PHOTO[name], TONES[tone] if weathered else 1.0)
	else:
		var plain := StandardMaterial3D.new()
		var c: Color = PLAIN.get(name, Color(0.5, 0.45, 0.4))
		plain.albedo_color = c
		if c.a < 1.0:
			plain.transparency = BaseMaterial3D.TRANSPARENCY_ALPHA
		m = plain
	_cache[key] = m
	return m


## Weathered wood for fences, woodpiles and the like.
func wood_material() -> Material:
	return material_for("Trim (photo)")


func build(street: Dictionary) -> void:
	var scene: PackedScene = load(HOUSE_SCENE)
	var rng := RandomNumberGenerator.new()
	rng.seed = 1901
	for plot in street.plots:
		var root: Node3D = scene.instantiate()
		var centre: Vector2 = (plot.house as Rect2).get_center()
		root.position = Vector3(centre.x, 0, centre.y)
		# The model faces +z (south). Houses across the road turn to face north.
		if not plot.north:
			root.rotation.y = PI
		# Variety: the player's house is the painted one; others are mostly
		# plain, in slightly different tones, some built the other way round.
		var painted: bool = plot.is_player or rng.randf() < 0.25
		var tone := rng.randi_range(0, TONES.size() - 1)
		if not plot.is_player and rng.randf() < 0.5:
			root.scale.x = -1.0
		root.rotation.y += rng.randf_range(-0.03, 0.03)
		add_child(root)

		# This house's own glass, so its windows can glow on their own.
		var glass := StandardMaterial3D.new()
		glass.albedo_color = PLAIN["Glass"]
		glass.transparency = BaseMaterial3D.TRANSPARENCY_ALPHA
		glass.emission_enabled = true
		glass.emission = Color(1.0, 0.72, 0.38)
		glass.emission_energy_multiplier = 0.0

		var parts := {}
		for mi in root.find_children("*", "MeshInstance3D", true, false):
			parts[mi.name] = mi
			for i in mi.mesh.get_surface_count():
				var src: Material = mi.mesh.surface_get_material(i)
				var name := src.resource_name if src else ""
				mi.set_surface_override_material(i, glass if name == "Glass" else material_for(name, painted, tone))
		if parts.has("Walls_Cutaway"):
			parts["Walls_Cutaway"].visible = false

		var smoke := _make_smoke()
		smoke.position = root.transform * CHIMNEY
		add_child(smoke)
		_by_plot[plot.id] = houses.size()
		houses.append({"root": root, "parts": parts, "glass": glass, "smoke": smoke, "plot": plot, "open": false})


## The house standing on a plot.
func index_for_plot(plot_id: String) -> int:
	return _by_plot.get(plot_id, -1)


func _make_smoke() -> GPUParticles3D:
	var p := GPUParticles3D.new()
	p.amount = 18
	p.lifetime = 5.0
	p.preprocess = 5.0
	var process := ParticleProcessMaterial.new()
	process.direction = Vector3(0.3, 1, 0)
	process.spread = 12.0
	process.initial_velocity_min = 0.5
	process.initial_velocity_max = 0.8
	process.gravity = Vector3(0.25, 0.15, 0)
	process.scale_min = 0.6
	process.scale_max = 1.0
	var grow := Curve.new()
	grow.add_point(Vector2(0, 0.4))
	grow.add_point(Vector2(1, 2.4))
	var grow_tex := CurveTexture.new()
	grow_tex.curve = grow
	process.scale_curve = grow_tex
	var fade := Gradient.new()
	fade.set_color(0, Color(0.75, 0.75, 0.75, 0.35))
	fade.set_color(1, Color(0.8, 0.8, 0.8, 0.0))
	var fade_tex := GradientTexture1D.new()
	fade_tex.gradient = fade
	process.color_ramp = fade_tex
	p.process_material = process
	var puff := SphereMesh.new()
	puff.radius = 0.35
	puff.height = 0.7
	puff.radial_segments = 8
	puff.rings = 4
	var mat := StandardMaterial3D.new()
	mat.transparency = BaseMaterial3D.TRANSPARENCY_ALPHA
	mat.vertex_color_use_as_albedo = true
	mat.shading_mode = BaseMaterial3D.SHADING_MODE_UNSHADED
	puff.material = mat
	p.draw_pass_1 = puff
	p.emitting = false
	return p


## Opens or closes a house: roof and upper walls off, cutaway walls on.
func set_open(index: int, open: bool) -> void:
	var house: Dictionary = houses[index]
	if house.open == open:
		return
	house.open = open
	var parts: Dictionary = house.parts
	for name in ["Roof", "Walls_Full", "Windows"]:
		if parts.has(name):
			parts[name].visible = not open
	if parts.has("Walls_Cutaway"):
		parts["Walls_Cutaway"].visible = open


func is_open(index: int) -> bool:
	return index >= 0 and houses[index].open


## Smoke, window light and snow on the roofs. Houses with nobody living in
## them yet still show a little life, so the street doesn't look abandoned.
func update(households: Array, time: Dictionary) -> void:
	var darkness: float = 1.0 - time.light
	var lived := {}
	for h in households:
		lived[h.plot] = h
	var cold: bool = time.temperature < 14.0
	for house in houses:
		var h: Dictionary = lived.get(house.plot.id, {})
		var burning := false
		var home := true
		if h.is_empty():
			burning = cold or (time.hour >= 6 and time.hour <= 8) or (time.hour >= 17 and time.hour <= 19)
		else:
			home = not h.lost
			burning = home and h.stove_power > 0.05
		(house.smoke as GPUParticles3D).emitting = burning
		var glow := 0.0
		if home and darkness > 0.15:
			glow = darkness * (2.5 if burning else 1.2)
		(house.glass as StandardMaterial3D).emission_energy_multiplier = glow

	var snow: float = time.snow
	for m in _snowy:
		(m as ShaderMaterial).set_shader_parameter("snow", snow)
