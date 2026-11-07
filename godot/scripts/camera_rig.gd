extends Node3D
## The strategy camera: look down on the whole street, or swoop in close to
## a house. Closer in, the view tilts towards the horizon like Manor Lords.
##
##   Move: WASD / arrows, or drag      Rotate: Q / E, or right-drag
##   Zoom: scroll, trackpad pinch or two-finger scroll

signal clicked(ground: Vector3)

const MIN_DISTANCE := 7.0
const MAX_DISTANCE := 170.0
const PAN_SPEED := 0.9      # fraction of the view distance per second
const ROTATE_SPEED := 1.6   # radians per second

var target := Vector3.ZERO
var distance := 90.0
var yaw := 0.0
var camera: Camera3D

var _home_target := Vector3.ZERO
var _home_distance := 90.0
var _home_yaw := 0.0
var _dragging := false
var _drag_moved := false
var _drag_start := Vector2.ZERO
var _rotating := false
var _ui_blocks: Callable = func(_p): return false


func _ready() -> void:
	camera = Camera3D.new()
	camera.fov = 40.0
	camera.near = 0.2
	camera.far = 1500.0
	add_child(camera)


## A function that says whether a screen point is over the interface.
func set_ui_check(check: Callable) -> void:
	_ui_blocks = check


func frame(centre: Vector3, view_distance: float, view_yaw := 0.0) -> void:
	_home_target = centre
	_home_distance = view_distance
	_home_yaw = view_yaw
	reset()


func reset() -> void:
	target = _home_target
	distance = _home_distance
	yaw = _home_yaw


## 1 when zoomed right in, 0 at the widest view.
func closeness() -> float:
	return 1.0 - clampf((distance - MIN_DISTANCE) / (MAX_DISTANCE - MIN_DISTANCE), 0.0, 1.0)


func _pitch() -> float:
	# Steep from high up, lower and more cinematic close in.
	return lerpf(deg_to_rad(62.0), deg_to_rad(28.0), pow(closeness(), 1.6))


func _process(delta: float) -> void:
	var move := Vector2.ZERO
	if Input.is_key_pressed(KEY_W) or Input.is_key_pressed(KEY_UP): move.y -= 1
	if Input.is_key_pressed(KEY_S) or Input.is_key_pressed(KEY_DOWN): move.y += 1
	if Input.is_key_pressed(KEY_A) or Input.is_key_pressed(KEY_LEFT): move.x -= 1
	if Input.is_key_pressed(KEY_D) or Input.is_key_pressed(KEY_RIGHT): move.x += 1
	if move != Vector2.ZERO:
		var step := move.normalized() * distance * PAN_SPEED * delta
		target += Vector3(step.x, 0, step.y).rotated(Vector3.UP, yaw)
	if Input.is_key_pressed(KEY_Q): yaw += ROTATE_SPEED * delta
	if Input.is_key_pressed(KEY_E): yaw -= ROTATE_SPEED * delta

	var pitch := _pitch()
	var offset := Vector3(0, sin(pitch), cos(pitch)) * distance
	camera.position = target + offset.rotated(Vector3.UP, yaw)
	camera.look_at(target, Vector3.UP)


func _zoom(factor: float) -> void:
	distance = clampf(distance * factor, MIN_DISTANCE, MAX_DISTANCE)


func _unhandled_input(event: InputEvent) -> void:
	if event is InputEventMouseButton:
		var mb := event as InputEventMouseButton
		if _ui_blocks.call(mb.position):
			return
		match mb.button_index:
			MOUSE_BUTTON_WHEEL_UP:
				if mb.pressed: _zoom(0.88)
			MOUSE_BUTTON_WHEEL_DOWN:
				if mb.pressed: _zoom(1.0 / 0.88)
			MOUSE_BUTTON_LEFT, MOUSE_BUTTON_MIDDLE:
				if mb.pressed:
					_dragging = true
					_drag_moved = false
					_drag_start = mb.position
				else:
					if _dragging and not _drag_moved and mb.button_index == MOUSE_BUTTON_LEFT:
						var ground = ground_point(mb.position)
						if ground != null:
							clicked.emit(ground)
					_dragging = false
			MOUSE_BUTTON_RIGHT:
				_rotating = mb.pressed
	elif event is InputEventMouseMotion:
		var mm := event as InputEventMouseMotion
		if _dragging:
			if mm.position.distance_to(_drag_start) > 4.0:
				_drag_moved = true
			if _drag_moved:
				# Keep the ground under the cursor.
				var scale := distance / 600.0
				target -= Vector3(mm.relative.x, 0, mm.relative.y).rotated(Vector3.UP, yaw) * scale
		elif _rotating:
			yaw -= mm.relative.x * 0.006
	elif event is InputEventMagnifyGesture:
		_zoom(1.0 / (event as InputEventMagnifyGesture).factor)
	elif event is InputEventPanGesture:
		# Two-finger trackpad scroll zooms, like a mouse wheel.
		_zoom(1.0 + (event as InputEventPanGesture).delta.y * 0.05)


## Where a screen point meets the ground, or null.
func ground_point(screen: Vector2):
	var origin := camera.project_ray_origin(screen)
	var dir := camera.project_ray_normal(screen)
	return Plane(Vector3.UP, 0.0).intersects_ray(origin, dir)
