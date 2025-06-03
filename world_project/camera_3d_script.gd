extends Camera3D

@export var planet: NodePath = NodePath("Planet") # Chemin relatif vers le nœud Planet
@export var rotate_around_planet_speed: float = 1.0 # Vitesse de rotation autour de la planète
@export var rotate_cam_speed: float = 1.0 # Vitesse de rotation de la caméra
@export var radius_mult: float = 1.1
@export var move_forward_backward_speed = 0.1
@export var zoom_sensitivity = 0.5
@export var move_factor_from_zoom = 7

var planet_node: Node3D
var camera : Camera3D

var distance = 1.0
var theta = PI/8.0
var phi = 0.0
var pitch = 0.0
var yaw = 0.0

var move = 0.0

var _delta_for_zoom = 1/60

func _ready():
	camera = self
	planet_node = get_node(planet)
	distance = planet_node.get("radius")

func _input(event):
	if event is InputEventMouseButton:
		if event.button_index == MOUSE_BUTTON_WHEEL_UP:
			# Zoom in (approcher) - plus sensible quand on est loin
			move -= move_forward_backward_speed * _delta_for_zoom * zoom_sensitivity * (1 + move)
		elif event.button_index == MOUSE_BUTTON_WHEEL_DOWN:
			# Zoom out (reculer) - moins sensible quand on est loin
			move += move_forward_backward_speed * _delta_for_zoom * zoom_sensitivity * (1 + move)
		
		# Appliquer les contraintes
		move = clamp(move, 1/radius_mult + 0.03, 1/radius_mult + 0.3)

func _process(delta):
	_delta_for_zoom = delta
	
	var factor = (move - 1/radius_mult) * move_factor_from_zoom
	if Input.is_action_pressed("up_rotate_around_planet"):
		theta += rotate_around_planet_speed * delta * factor * factor
	if Input.is_action_pressed("down_rotate_around_planet"):
		theta -= rotate_around_planet_speed * delta * factor
	if Input.is_action_pressed("left_rotate_around_planet"):
		phi -= rotate_around_planet_speed * delta * factor
	if Input.is_action_pressed("right_rotate_around_planet"):
		phi += rotate_around_planet_speed * delta * factor
	if Input.is_action_pressed("up_cam"):
		pitch += rotate_around_planet_speed * delta
	if Input.is_action_pressed("down_cam"):
		pitch -= rotate_around_planet_speed * delta
	if Input.is_action_pressed("left_cam"):
		yaw += rotate_around_planet_speed * delta
	if Input.is_action_pressed("right_cam"):
		yaw -= rotate_around_planet_speed * delta
	if Input.is_action_pressed("move_forward"):
		move -= move_forward_backward_speed * delta * zoom_sensitivity * (1 + move)
	if Input.is_action_pressed("move_backward"):
		move += move_forward_backward_speed * delta * zoom_sensitivity * (1 + move)
	
	theta = clamp(theta, -PI/2.0, PI/2.0)
	# phi = clamp(phi, -PI, PI)
	yaw = clamp(yaw, -PI/12.0, PI/12.0)
	pitch = clamp(pitch, -PI/8.0, PI/6.0)
	
	move = clamp(move, 1/radius_mult + 0.05, 1/radius_mult + 0.2)
	
	update_camera()
	
func update_camera():
	var offset = Vector3(sin(phi) * cos(theta),sin(theta),cos(phi) * cos(theta)) * distance * move * radius_mult
	
	camera.global_transform.origin = planet_node.global_transform.origin + offset
	camera.look_at(planet_node.global_transform.origin, Vector3.UP)
	camera.rotate_object_local(Vector3(1, 0, 0), pitch)
	camera.rotate_object_local(Vector3(0, 1, 0), yaw)

	
