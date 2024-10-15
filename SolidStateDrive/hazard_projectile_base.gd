class_name Projectile extends Hazard
	
## The 3D version of this projectile
@export var projectile3D: PackedScene
var proj3DInstance: Node3D

## Evolution of the speed of the bullet over time once it is shot
@export var speedCurve: Curve
## Represents value 1 in the speed curve
@export var maxSpeed: float
## Time in seconds it takes the bullet to reach max speed
@export var rampUpTime: float
@export var lifeTime: float = 5



@onready var hit_component = $HitComponent

## 3D Global position of target
var targetPos: Vector2 = PlayerVariables.player_node.global_position
var target3DPos: Vector3

## 3D Global position of the bullet when shot
var originPos: Vector2
var origin3DPos: Vector3

# Internal variables to calculate movement
var direction: Vector2
var direction3D: Vector3

var shotTime: float = 0
	
## Initializes the bullet
func _ready():	
	
	# Instantiate 3D projectile
	proj3DInstance = projectile3D.instantiate()
	PlayerVariables.world3d_node.add_child(proj3DInstance)
	
	_init_target()	
	set3DProjectileTransform(true)

# New target should be global 3D world position 
func set_target(newTarget: Vector2):
	targetPos =  newTarget
	_init_target()
	
func _init_target():
	# Configure transform of 2D projectile
	originPos = PlayerVariables.pos2dglobal_to_3dglobal(position)
	direction = (targetPos - originPos).normalized()
	rotation = direction.angle()
	
	# Configure transform of 3D projectile
	var result = PlayerVariables.IntersectionManager.pos2DGlobalto3DGlobal(PlayerVariables.pos2dglobal_to_3dglobal(position))
	if not result is Vector3:
		_on_died()
		return
	origin3DPos = result
	target3DPos = PlayerVariables.IntersectionManager.pos2DGlobalto3DGlobal(targetPos)
	direction3D = (target3DPos - origin3DPos).normalized()	

func _physics_process(delta):
	var speed = speedCurve.sample(shotTime/rampUpTime) * maxSpeed
	position += direction * speed * delta
	
	proj3DInstance.get_child(0).rotate_object_local(Vector3.UP, delta * 10)
	set3DProjectileTransform()
	
	shotTime += delta	
	if shotTime > lifeTime:
		_on_died()
	queue_redraw()
		
func _on_died():
	proj3DInstance.queue_free()
	queue_free()
	
func set3DProjectileTransform(isInit: bool = false):
	var projectileGlobalTransform = get_global_self_transform()
	var result = PlayerVariables.IntersectionManager.pos2DGlobalto3DGlobal(projectileGlobalTransform.global_position)
	if not result is Vector3:
		_on_died()
		return
	proj3DInstance.global_position = result
	if isInit:
		proj3DInstance.look_at(proj3DInstance.global_position + direction3D)
	
func _draw():
	if not PlayerVariables.IntersectionManager.debugEnabled:
		return
		
	var origin = to_local(originPos)
	var target = to_local(targetPos)
	var origin3D = to_local(PlayerVariables.IntersectionManager.pos3DGlobalto2DGlobal(origin3DPos))
	var target3D = to_local(PlayerVariables.IntersectionManager.pos3DGlobalto2DGlobal(origin3DPos + direction3D))
	
	draw_circle(origin3D, 8, Color.BLUE)
	draw_line(origin3D, target3D, Color.RED, 4)
	draw_circle(origin, 5, Color.AQUA)
	draw_line(origin, target, Color.WHITE_SMOKE, 2)

func remove_from_tree():
	proj3DInstance.get_parent().remove_child(proj3DInstance)
	get_parent().remove_child(self)
	
func add_to_tree():
	PlayerVariables.world2d_node.add_child(self)
	PlayerVariables.world3d_node.add_child(proj3DInstance)
	
func get_global_self_transform():
	return self
