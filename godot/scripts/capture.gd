extends Node
## Screenshot mode, for checking how the game looks. Launch with
##
##   Godot --path . -- --capture=<folder> [--shots=a,b]
##
## It sets up each shot (season, time of day, camera), waits for the image to
## settle, saves a PNG and quits when done.

# name: [season, hour, camera distance, yaw degrees, focus ("street" or "home"), offset]
const SHOTS := {
	"summer_aerial": [1, 17.5, 72.0, 62.0, "street", Vector2(0, 0)],
	"summer_noon": [1, 12.0, 58.0, 35.0, "street", Vector2(0, 0)],
	"house_close": [1, 16.5, 34.0, 50.0, "home", Vector2(4, 14)],
	"inside": [1, 12.5, 24.0, 62.0, "home", Vector2(0, 0)],
	"winter": [3, 13.0, 68.0, 62.0, "street", Vector2(0, 0)],
	"night": [2, 21.5, 58.0, 62.0, "street", Vector2(0, 0)],
}

var main: Node
var folder := ""
var only: Array = []


func run(owner_main: Node, out_folder: String, shots: Array) -> void:
	main = owner_main
	folder = out_folder
	only = shots
	DirAccess.make_dir_recursive_absolute(folder)
	await get_tree().process_frame
	for name in SHOTS:
		if not only.is_empty() and not name in only:
			continue
		await _shot(name, SHOTS[name])
	get_tree().quit()


func _shot(name: String, spec: Array) -> void:
	main.paused = true
	main.sim.restart()
	main.sim.jump_to_season(spec[0])
	# jump_to_season lands at 06:00 on day 1; move on a few days and to the hour.
	main.sim.advance(4 * 1440 + int((float(spec[1]) - 6.0) * 60.0))
	main.sim.take_events()

	var cam = main.get_node_or_null("CameraRig")
	var focus: Vector3 = cam._home_target
	if spec[4] == "home":
		var plot: Dictionary = main.street.plots[0]
		for p in main.street.plots:
			if p.is_player:
				plot = p
		var c: Vector2 = (plot.house as Rect2).get_center()
		focus = Vector3(c.x, 0, c.y)
	var offset: Vector2 = spec[5]
	cam.target = focus + Vector3(offset.x, 0, offset.y)
	cam.distance = spec[2]
	cam.yaw = deg_to_rad(spec[3])

	# Let shadows, ambient occlusion and particles settle.
	for i in 90:
		await get_tree().process_frame
	var image := get_viewport().get_texture().get_image()
	image.save_png(folder.path_join(name + ".png"))
	print("captured ", name)
