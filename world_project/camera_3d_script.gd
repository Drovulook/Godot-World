extends Camera3D

@export var planet: NodePath = NodePath("Planet") # Chemin relatif vers le nœud Planet
@export var rotate_around_planet_speed: float = 1.0 # Vitesse de rotation autour de la planète
@export var rotate_cam_speed: float = 1.0 # Vitesse de rotation de la caméra
@export var radius_mult: float = 1.1

var planet_node: Node3D
var camera : Camera3D

var distance = 1.0
var theta = PI/8.0
var phi = 0.0
var pitch = 0.0
var yaw = 0.0

func _ready():
	camera = self
	planet_node = get_node(planet)
	distance = planet_node.get("radius")
	
func _process(delta):
	if Input.is_action_pressed("up_rotate_around_planet"):
		theta += rotate_around_planet_speed * delta
	if Input.is_action_pressed("down_rotate_around_planet"):
		theta -= rotate_around_planet_speed * delta
	if Input.is_action_pressed("left_rotate_around_planet"):
		phi -= rotate_around_planet_speed * delta
	if Input.is_action_pressed("right_rotate_around_planet"):
		phi += rotate_around_planet_speed * delta
	if Input.is_action_pressed("up_cam"):
		pitch += rotate_around_planet_speed * delta
	if Input.is_action_pressed("down_cam"):
		pitch -= rotate_around_planet_speed * delta
	if Input.is_action_pressed("left_cam"):
		yaw += rotate_around_planet_speed * delta
	if Input.is_action_pressed("right_cam"):
		yaw -= rotate_around_planet_speed * delta
	
	theta = clamp(theta, -PI/2.0, PI/2.0)
	yaw = clamp(yaw, -PI/14.0, PI/14.0)
	pitch = clamp(pitch, -PI/8.0, PI/8.0)
	
	update_camera()
	
func update_camera():
	var offset = Vector3(sin(phi) * cos(theta),sin(theta),cos(phi) * cos(theta)) * distance * radius_mult
	
	camera.global_transform.origin = planet_node.global_transform.origin + offset
	camera.look_at(planet_node.global_transform.origin, Vector3.UP)
	camera.rotate_object_local(Vector3(1, 0, 0), pitch)
	camera.rotate_object_local(Vector3(0, 1, 0), yaw)

	
