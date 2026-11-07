extends Node3D
## The houses: the Blender model on every plot, with photo-textured
## materials, chimney smoke while the stove burns, windows that glow at
## night, and the cutaway that lifts the roof and lowers the walls so people
## inside can be seen.

const HOUSE_SCENE := "res://assets/models/lind_house.glb"
const TEXTURES := "res://assets/textures/"
const CHIMNEY := Vector3(-1.0, 7.05, 0.45)  # chimney top, relative to a north-side house

# Blender material name -> how to draw it in Godot.
# Photo textures: [texture, real size in metres, tint]. Plain: a colour.
const PHOTO := {
	"Logs (photo)": ["weathered_brown_planks", 2.4, Color(0.62, 0.6, 0.58)],
	"Trim (photo)": ["weathered_brown_planks", 2.0, Color(0.5, 0.48, 0.46)],
	"Shingles (photo)": ["roof_09", 3.2, Color(0.62, 0.6, 0.58)],
	"Shingles dark (photo)": ["roof_09", 3.0, Color(0.46, 0.45, 0.43)],
	"Shingles light (photo)": ["roof_09", 3.4, Color(0.74, 0.72, 0.69)],
	"Fieldstone (photo)": ["rustic_stone_wall", 1.3, Color(0.75, 0.74, 0.72)],
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
const ROOF_MATERIALS := ["Shingles (photo)", "Shingles dark (photo)", "Shingles light (photo)"]

var houses := []  # one per household: {root, parts, glass, smoke, plot, open}
var _shared := {}
var _roof_base := {}


func material_for(name: String) -> Material:
	if _shared.has(name):
		return _shared[name]
	var m := StandardMaterial3D.new()
	if PHOTO.has(name):
		var spec: Array = PHOTO[name]
		var id: String = spec[0]
		m.albedo_texture = load(TEXTURES + id + "/" + id + "_diff_2k.jpg")
		m.normal_enabled = true
		m.normal_texture = load(TEXTURES + id + "/" + id + "_nor_gl_2k.jpg")
		m.roughness_texture = load(TEXTURES + id + "/" + id + "_rough_2k.jpg")
		m.roughness_texture_channel = BaseMaterial3D.TEXTURE_CHANNEL_GRAYSCALE
		m.uv1_triplanar = true
		m.uv1_world_triplanar = true
		m.uv1_scale = Vector3.ONE / float(spec[1])
		m.albedo_color = spec[2]
	elif PLAIN.has(name):
		var c: Color = PLAIN[name]
		m.albedo_color = c
		if c.a < 1.0:
			m.transparency = BaseMaterial3D.TRANSPARENCY_ALPHA
	else:
		m.albedo_color = Color(0.5, 0.45, 0.4)
	_shared[name] = m
	if name in ROOF_MATERIALS:
		_roof_base[name] = m.albedo_color
	return m


## Weathered wood for fences, woodpiles and the like.
func wood_material() -> Material:
	return material_for("Trim (photo)")


func build(street: Dictionary, households: Array) -> void:
	var scene: PackedScene = load(HOUSE_SCENE)
	for h in households:
		var plot: Dictionary = {}
		for p in street.plots:
			if p.id == h.plot:
				plot = p
		var root: Node3D = scene.instantiate()
		var centre: Vector2 = (plot.house as Rect2).get_center()
		root.position = Vector3(centre.x, 0, centre.y)
		# The model faces +z (south). Houses across the road turn to face north.
		if not plot.north:
			root.rotation.y = PI
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
				mi.set_surface_override_material(i, glass if name == "Glass" else material_for(name))
		if parts.has("Walls_Cutaway"):
			parts["Walls_Cutaway"].visible = false

		var smoke := _make_smoke()
		smoke.position = root.transform * CHIMNEY
		add_child(smoke)
		houses.append({"root": root, "parts": parts, "glass": glass, "smoke": smoke, "plot": plot, "open": false})


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
	return houses[index].open


## Smoke, window light and snow on the roofs.
func update(households: Array, time: Dictionary) -> void:
	var darkness: float = 1.0 - time.light
	for i in houses.size():
		var h: Dictionary = households[i]
		var house: Dictionary = houses[i]
		var lived_in: bool = not h.lost
		(house.smoke as GPUParticles3D).emitting = lived_in and h.stove_power > 0.05
		var glow := 0.0
		if lived_in and darkness > 0.15:
			glow = darkness * (2.5 if h.stove_power > 0.05 else 1.2)
		(house.glass as StandardMaterial3D).emission_energy_multiplier = glow

	var snow: float = time.snow
	for name in _roof_base:
		(_shared[name] as StandardMaterial3D).albedo_color = (_roof_base[name] as Color).lerp(Color(0.95, 0.96, 0.98), snow)
