extends CanvasLayer
## The interface: top bar, your household with job buttons, the selected
## person, warnings, the event log, the debug panel and the game over screen.
## It only shows what the simulation reports and passes on the player's
## choices through `main`.

const JOBS := ["Firewood", "Garden", "Preserve", "Rest"]
const PLACES := ["the house", "the garden", "the woodpile", "the yard"]
const ACCENT := Color8(226, 186, 96)
const MUTED := Color8(176, 168, 150)
const BAD := Color8(220, 110, 90)

var main: Node

var _date: Label
var _place: Label
var _temp: Label
var _weather: Label
var _speed_buttons := []
var _season_bar: ColorRect
var _warnings: HBoxContainer
var _house_box: VBoxContainer
var _house_lines := {}
var _member_rows := []
var _person_panel: PanelContainer
var _person := {}
var _log_box: VBoxContainer
var _log := []
var _debug_panel: PanelContainer
var _debug_lines: Label
var _game_over: Control
var _game_over_text: Label


func _style(alpha := 0.78) -> StyleBoxFlat:
	var s := StyleBoxFlat.new()
	s.bg_color = Color(0.06, 0.07, 0.06, alpha)
	s.set_corner_radius_all(8)
	s.content_margin_left = 14
	s.content_margin_right = 14
	s.content_margin_top = 10
	s.content_margin_bottom = 10
	return s


func _label(text := "", size := 15, colour := Color(0.93, 0.9, 0.85)) -> Label:
	var l := Label.new()
	l.text = text
	l.add_theme_font_size_override("font_size", size)
	l.add_theme_color_override("font_color", colour)
	return l


func _panel(parent: Control) -> PanelContainer:
	var p := PanelContainer.new()
	p.add_theme_stylebox_override("panel", _style())
	parent.add_child(p)
	return p


func setup(owner_main: Node, households: Array, player: int, speeds: Array) -> void:
	main = owner_main
	var root := Control.new()
	root.set_anchors_preset(Control.PRESET_FULL_RECT)
	root.mouse_filter = Control.MOUSE_FILTER_IGNORE
	add_child(root)

	_build_top_bar(root, speeds)
	_build_warnings(root)
	_build_household(root, households[player])
	_build_person(root)
	_build_log(root)
	_build_help(root)
	_build_debug(root)
	_build_game_over(root)


# --- Top bar ---------------------------------------------------------------

func _build_top_bar(root: Control, speeds: Array) -> void:
	var bar := PanelContainer.new()
	bar.add_theme_stylebox_override("panel", _style(0.6))
	bar.set_anchors_preset(Control.PRESET_TOP_WIDE)
	bar.custom_minimum_size.y = 70
	root.add_child(bar)
	var row := HBoxContainer.new()
	bar.add_child(row)

	var left := VBoxContainer.new()
	_place = _label("", 22)
	_date = _label("", 15, MUTED)
	left.add_child(_place)
	left.add_child(_date)
	row.add_child(left)
	row.add_child(_spacer())

	var middle := VBoxContainer.new()
	_temp = _label("", 24)
	_weather = _label("", 13, MUTED)
	middle.add_child(_temp)
	middle.add_child(_weather)
	row.add_child(middle)
	row.add_child(_spacer())

	var pause := Button.new()
	pause.text = "Pause"
	pause.toggle_mode = true
	pause.custom_minimum_size = Vector2(64, 34)
	pause.pressed.connect(func(): main.set_paused(not main.paused))
	row.add_child(pause)
	_speed_buttons.append(pause)
	for i in speeds.size():
		var b := Button.new()
		b.text = "%sx" % str(speeds[i])
		b.toggle_mode = true
		b.custom_minimum_size = Vector2(52, 34)
		b.pressed.connect(func(): main.set_speed(i))
		row.add_child(b)
		_speed_buttons.append(b)

	_season_bar = ColorRect.new()
	_season_bar.color = ACCENT
	_season_bar.position = Vector2(0, 70)
	_season_bar.size = Vector2(0, 3)
	_season_bar.mouse_filter = Control.MOUSE_FILTER_IGNORE
	root.add_child(_season_bar)


func _spacer() -> Control:
	var s := Control.new()
	s.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	return s


func _build_warnings(root: Control) -> void:
	_warnings = HBoxContainer.new()
	_warnings.set_anchors_preset(Control.PRESET_CENTER_TOP)
	_warnings.position = Vector2(0, 84)
	_warnings.alignment = BoxContainer.ALIGNMENT_CENTER
	_warnings.add_theme_constant_override("separation", 8)
	_warnings.mouse_filter = Control.MOUSE_FILTER_IGNORE
	root.add_child(_warnings)


# --- Household and person ----------------------------------------------------

func _build_household(root: Control, household: Dictionary) -> void:
	var panel := _panel(root)
	panel.position = Vector2(16, 84)
	panel.custom_minimum_size.x = 360
	_house_box = VBoxContainer.new()
	panel.add_child(_house_box)
	_house_box.add_child(_label(household.name, 21, ACCENT))
	for key in ["food", "storage", "firewood", "house", "garden"]:
		var l := _label("", 14)
		_house_lines[key] = l
		_house_box.add_child(l)
	_house_box.add_child(HSeparator.new())

	for i in household.members.size():
		var person: Dictionary = household.members[i]
		var top := HBoxContainer.new()
		var name := Button.new()
		name.text = person.name
		name.flat = true
		name.alignment = HORIZONTAL_ALIGNMENT_LEFT
		name.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		name.pressed.connect(func(): main.select(Vector2i(main.player, i)))
		top.add_child(name)
		var state := _label("", 13, BAD)
		top.add_child(state)
		var health := ProgressBar.new()
		health.custom_minimum_size = Vector2(100, 12)
		health.show_percentage = false
		health.size_flags_vertical = Control.SIZE_SHRINK_CENTER
		top.add_child(health)
		_house_box.add_child(top)

		var jobs := HBoxContainer.new()
		var group := ButtonGroup.new()
		var job_buttons := []
		for j in JOBS.size():
			var b := Button.new()
			b.text = JOBS[j]
			b.toggle_mode = true
			b.button_group = group
			b.size_flags_horizontal = Control.SIZE_EXPAND_FILL
			b.add_theme_font_size_override("font_size", 13)
			b.pressed.connect(func(): main.set_job(i, j))
			jobs.add_child(b)
			job_buttons.append(b)
		_house_box.add_child(jobs)
		_member_rows.append({"state": state, "health": health, "jobs": job_buttons, "job_row": jobs})


func _build_person(root: Control) -> void:
	_person_panel = _panel(root)
	_person_panel.custom_minimum_size.x = 360
	_person_panel.visible = false
	var box := VBoxContainer.new()
	_person_panel.add_child(box)
	var top := HBoxContainer.new()
	_person.name = _label("", 19)
	_person.name.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	top.add_child(_person.name)
	var follow := Button.new()
	follow.text = "Follow"
	follow.toggle_mode = true
	follow.pressed.connect(func(): main.following = follow.button_pressed)
	_person.follow = follow
	top.add_child(follow)
	var close := Button.new()
	close.text = "x"
	close.pressed.connect(func(): main.select(Vector2i(-1, -1)))
	top.add_child(close)
	box.add_child(top)
	_person.role = _label("", 13, MUTED)
	_person.activity = _label("", 15, ACCENT)
	box.add_child(_person.role)
	box.add_child(_person.activity)
	var order_row := HBoxContainer.new()
	_person.order = _label("", 13)
	_person.order.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	order_row.add_child(_person.order)
	var cancel := Button.new()
	cancel.text = "Cancel"
	cancel.pressed.connect(func(): main.cancel_order())
	_person.cancel = cancel
	order_row.add_child(cancel)
	box.add_child(order_row)
	for stat in ["Health", "Warmth", "Fed", "Morale"]:
		var row := HBoxContainer.new()
		var l := _label(stat, 13, MUTED)
		l.custom_minimum_size.x = 70
		row.add_child(l)
		var bar := ProgressBar.new()
		bar.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		bar.custom_minimum_size.y = 14
		row.add_child(bar)
		box.add_child(row)
		_person[stat] = bar


# --- Log, help, debug, game over ---------------------------------------------

func _build_log(root: Control) -> void:
	var panel := _panel(root)
	panel.set_anchors_preset(Control.PRESET_BOTTOM_RIGHT)
	panel.grow_horizontal = Control.GROW_DIRECTION_BEGIN
	panel.grow_vertical = Control.GROW_DIRECTION_BEGIN
	panel.position = Vector2(-16, -48)
	panel.custom_minimum_size = Vector2(480, 0)
	_log_box = VBoxContainer.new()
	panel.add_child(_log_box)
	_log_box.add_child(_label("What has happened", 15, ACCENT))


func _build_help(root: Control) -> void:
	var bar := PanelContainer.new()
	bar.add_theme_stylebox_override("panel", _style(0.55))
	bar.set_anchors_preset(Control.PRESET_BOTTOM_WIDE)
	bar.grow_vertical = Control.GROW_DIRECTION_BEGIN
	root.add_child(bar)
	bar.add_child(_label("Click a person, then click your plot to send them   ·   Move: WASD or drag   ·   Rotate: Q/E or right-drag   ·   Zoom: scroll or pinch   ·   Reset: R   ·   Pause: Space   ·   Speed: 1-4   ·   Debug: `", 13, MUTED))


func _build_debug(root: Control) -> void:
	_debug_panel = _panel(root)
	_debug_panel.set_anchors_preset(Control.PRESET_TOP_RIGHT)
	_debug_panel.grow_horizontal = Control.GROW_DIRECTION_BEGIN
	_debug_panel.position = Vector2(-16, 84)
	_debug_panel.custom_minimum_size.x = 320
	_debug_panel.visible = false
	var box := VBoxContainer.new()
	_debug_panel.add_child(box)
	box.add_child(_label("Debug", 19, ACCENT))
	box.add_child(_label("Jump to the start of", 13, MUTED))
	var seasons := HBoxContainer.new()
	for s in ["Spring", "Summer", "Autumn", "Winter"].size():
		var b := Button.new()
		b.text = ["Spring", "Summer", "Autumn", "Winter"][s]
		b.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		b.pressed.connect(func(): main.sim.jump_to_season(s))
		seasons.add_child(b)
	box.add_child(seasons)
	for item in [["Food", true], ["Firewood", false]]:
		var row := HBoxContainer.new()
		var l := _label(item[0], 14, MUTED)
		l.custom_minimum_size.x = 80
		row.add_child(l)
		for change in [-10.0, 10.0]:
			var b := Button.new()
			b.text = "%+d" % int(change)
			b.size_flags_horizontal = Control.SIZE_EXPAND_FILL
			var food: bool = item[1]
			b.pressed.connect(func(): main.sim.adjust_stores(main.player, change if food else 0.0, 0.0 if food else change))
			row.add_child(b)
		var empty := Button.new()
		empty.text = "Empty"
		var is_food: bool = item[1]
		empty.pressed.connect(func(): main.empty_store(is_food))
		row.add_child(empty)
		box.add_child(row)
	var ripen := Button.new()
	ripen.text = "Sow and ripen the garden now"
	ripen.pressed.connect(func(): main.sim.ripen_garden(main.player))
	box.add_child(ripen)
	var gi := Button.new()
	gi.text = "Toggle global illumination (slower)"
	gi.pressed.connect(func(): main.toggle_gi())
	box.add_child(gi)
	_debug_lines = _label("", 13)
	box.add_child(_debug_lines)


func _build_game_over(root: Control) -> void:
	_game_over = ColorRect.new()
	(_game_over as ColorRect).color = Color(0, 0, 0, 0.6)
	_game_over.set_anchors_preset(Control.PRESET_FULL_RECT)
	_game_over.visible = false
	root.add_child(_game_over)
	var panel := _panel(_game_over)
	panel.set_anchors_preset(Control.PRESET_CENTER)
	panel.custom_minimum_size = Vector2(560, 0)
	var box := VBoxContainer.new()
	panel.add_child(box)
	_game_over_text = _label("", 16)
	_game_over_text.autowrap_mode = TextServer.AUTOWRAP_WORD
	box.add_child(_game_over_text)
	var again := Button.new()
	again.text = "Start again"
	again.pressed.connect(func(): main.restart())
	box.add_child(again)


func toggle_debug() -> void:
	_debug_panel.visible = not _debug_panel.visible


# --- Updating ---------------------------------------------------------------

func _date_text(time: Dictionary) -> String:
	return "Day %d of %s, %d" % [time.day, time.season_name, time.year]


func update(time: Dictionary, households: Array, warnings: Array) -> void:
	var player: int = main.player
	var h: Dictionary = households[player]

	_place.text = "%s, %s" % [main.street.street, main.street.country]
	_date.text = "%s  ·  %02d:%02d" % [_date_text(time), time.hour, time.minute]
	var temp: float = time.temperature
	_temp.text = "%+d°C" % roundi(temp)
	_temp.add_theme_color_override("font_color", Color8(170, 200, 236) if temp < 0 else Color8(236, 210, 160))
	_weather.text = "Snowing" if time.snowing else ("Freezing" if temp < 0 else "")
	_season_bar.size.x = get_viewport().get_visible_rect().size.x * float(time.season_progress)
	for i in _speed_buttons.size():
		var on: bool = main.paused if i == 0 else (not main.paused and main.speed_index == i - 1)
		_speed_buttons[i].set_pressed_no_signal(on)

	# Household
	var food_days: float = main.sim.food_days_left(player)
	var wood_days: float = main.sim.firewood_days_left(player)
	_house_lines.food.text = "Food: %d rations (about %d days)" % [roundi(h.fresh_food + h.preserved_food), roundi(food_days)]
	_house_lines.storage.text = "      %d fresh, %d preserved" % [roundi(h.fresh_food), roundi(h.preserved_food)]
	_house_lines.firewood.text = "Firewood: %d bundles (about %d days)" % [roundi(h.firewood), roundi(wood_days)]
	var stove := "stove cold" if h.firewood <= 0 else ("stove lit" if h.stove_power > 0.05 else "stove banked")
	_house_lines.house.text = "House: %d°C inside, %s" % [roundi(h.indoor_c), stove]
	_house_lines.garden.text = "Garden: " + _garden_text(h.garden, time)
	for i in _member_rows.size():
		var p: Dictionary = h.members[i]
		var row: Dictionary = _member_rows[i]
		row.health.value = p.health
		row.state.text = "died" if not p.alive else ("sick" if p.sick else "")
		row.job_row.visible = p.alive
		for j in JOBS.size():
			row.jobs[j].set_pressed_no_signal(p.job == j)

	# Selected person
	var sel: Vector2i = main.selected
	_person_panel.visible = sel.x >= 0
	if sel.x >= 0:
		_person_panel.position = Vector2(16, 84 + _house_box.get_parent().size.y + 10) if sel.x == player else Vector2(16, 84)
		var p: Dictionary = households[sel.x].members[sel.y]
		_person.name.text = p.name
		_person.role.text = "%s, age %d" % [p.role_name, p.age]
		_person.activity.text = p.activity if p.alive else "Died of %s" % p.cause
		var ordered: bool = p.order_until >= 0 and time.minute_total < p.order_until
		_person.cancel.visible = ordered
		if ordered:
			var left: int = p.order_until - time.minute_total
			_person.order.text = "Sent to %s, %dh %02dm left" % [PLACES[p.order_place], left / 60, left % 60]
		else:
			_person.order.text = "Job: %s. Click your plot to send them somewhere." % JOBS[p.job] if sel.x == player else ""
		_person.Health.value = p.health
		_person.Warmth.value = p.warmth
		_person.Fed.value = 100.0 - p.hunger
		_person.Morale.value = p.morale
		_person.follow.set_pressed_no_signal(main.following)

	_update_warnings(warnings, h)

	if _debug_panel.visible:
		_debug_lines.text = "Temperature %.1f°C, snow %d%%, daylight %d%%\nGarden sown %d%%, grown %d%%, weeded %d%%\nHarvest expected %d, in %d\nIndoor %.1f°C, stove %d%%" % [
			temp, roundi(time.snow * 100), roundi(time.light * 100),
			roundi(h.garden.planted * 100), roundi(h.garden.growth * 100), roundi(h.garden.care * 100),
			roundi(h.garden.expected), roundi(h.garden.harvested), h.indoor_c, roundi(h.stove_power * 100)]

	_game_over.visible = main.sim.player_lost()
	if _game_over.visible:
		var text := "%s did not survive.\n\n" % h.name
		for p in h.members:
			text += "%s, %d: died of %s.\n" % [p.name, p.age, p.cause]
		_game_over_text.text = text


func _garden_text(g: Dictionary, time: Dictionary) -> String:
	if g.planted <= 0.0:
		return "resting under the snow" if time.season == 3 else "not sown yet"
	if g.growth < 1.0:
		if g.planted < 1.0 and time.season == 0:
			return "%d%% sown, growing" % roundi(g.planted * 100)
		return "growing %d%%, weeded %d%%" % [roundi(g.growth * 100), roundi(g.care * 100)]
	if g.in_ground >= 1.0:
		return "ready, about %d rations to bring in" % roundi(g.in_ground)
	return "harvested, %d rations this year" % roundi(g.harvested)


func _update_warnings(warnings: Array, h: Dictionary) -> void:
	for c in _warnings.get_children():
		c.queue_free()
	for i in mini(warnings.size(), 4):
		var w: Dictionary = warnings[i]
		var text := ""
		var v: float = w.value
		match w.type:
			"firewood_out": text = "No firewood left: the stove is cold"
			"firewood_low": text = "Firewood will last %d more days at this rate" % maxi(1, roundi(v))
			"food_out": text = "No food left"
			"food_low": text = "Food will last %d more days" % maxi(1, roundi(v))
			"house_cold": text = "The house is cold: %d°C" % roundi(v)
			"person_sick": text = "%s is sick" % h.members[w.person].name
			"person_weak": text = "%s is very weak" % h.members[w.person].name
			"crop_ready": text = "The garden is ready: %d rations to harvest before the frost" % roundi(v)
			"storage_full": text = "No room for fresh food: %d rations rotting. Preserve it" % roundi(v)
		var pill := PanelContainer.new()
		var s := _style(0.92)
		s.bg_color = Color8(180, 60, 50) if w.urgent else Color8(200, 140, 50)
		s.content_margin_top = 4
		s.content_margin_bottom = 4
		pill.add_theme_stylebox_override("panel", s)
		pill.add_child(_label(text, 14))
		_warnings.add_child(pill)
	# Keep the row centred under the top bar.
	_warnings.position.x = (get_viewport().get_visible_rect().size.x - _warnings.size.x) * 0.5


## Turns simulation events into lines in the log.
func add_events(events: Array, households: Array, time_of: Callable) -> void:
	for e in events:
		var h: Dictionary = households[e.household] if e.household >= 0 else {}
		var p: Dictionary = h.members[e.person] if not h.is_empty() and e.person >= 0 else {}
		var v: float = e.value
		var text := ""
		var important := false
		match e.type:
			"new_season": text = "%s has come." % time_of.call(e.minute).season_name
			"new_year": text = "A new year begins: %d." % time_of.call(e.minute).year
			"fell_sick": text = "%s has fallen sick." % p.name; important = true
			"recovered": text = "%s is feeling better." % p.name
			"died": text = "%s has died of %s." % [p.name, p.cause]; important = true
			"food_ran_out": text = "%s have run out of food." % h.name; important = true
			"firewood_ran_out": text = "%s have run out of firewood." % h.name; important = true
			"household_lost": text = "%s are gone." % h.name; important = true
			"food_spoiled": text = "%d rations of fresh food have spoiled." % roundi(v)
			"planting_done": text = "The garden is sown."
			"crops_ready": text = "The garden is ready: about %d rations to harvest." % roundi(v); important = true
			"harvest_done": text = "The harvest is in: %d rations." % roundi(v)
			"frost": text = "Frost has ruined %d rations still in the ground." % roundi(v); important = true
			_: continue
		var t: Dictionary = time_of.call(e.minute)
		_log.push_front(["%s %d, %02d:%02d" % [t.season_name, t.day, t.hour, t.minute], text, important])
	while _log.size() > 7:
		_log.pop_back()
	for i in range(_log_box.get_child_count() - 1, 0, -1):
		_log_box.get_child(i).queue_free()
	for entry in _log:
		var row := HBoxContainer.new()
		var when := _label(entry[0], 13, MUTED)
		when.custom_minimum_size.x = 120
		row.add_child(when)
		row.add_child(_label(entry[1], 14, Color8(236, 170, 140) if entry[2] else Color(0.93, 0.9, 0.85)))
		_log_box.add_child(row)
	_log_box.get_parent().visible = not _log.is_empty()


func clear_log() -> void:
	_log.clear()
	add_events([], [], func(_m): return {})
