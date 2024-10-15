extends CharacterBody3D


@export var ACCELERATION = 1
@export var MAX_SPEED = 3
@export var ROTATION_SPEED = 3

enum{
	IDLE,
	CHASE
}

@onready var soft_collision = $SoftCollision

var state = IDLE
var dzplayer = null

func _physics_process(delta):
	match state:
		IDLE:
			seek_player()
				
		CHASE:
			var player = dzplayer
			if player != null:
				accelerate_towards_point(player.global_position, delta)
			else:
				state = IDLE
				
	if soft_collision.is_colliding():
		velocity += soft_collision.get_push_vector() * delta * 4
	move_and_slide()

func accelerate_towards_point(point, delta):
	var direction = global_position.direction_to(point)
	velocity = velocity.move_toward(direction * MAX_SPEED, ACCELERATION * delta)
	look_at(point)

func seek_player():
	if dzplayer != null:
		state = CHASE
	
func _on_player_detection_zone_body_entered(body):
	dzplayer = body
