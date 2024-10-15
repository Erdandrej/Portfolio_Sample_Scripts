class_name Player2D extends Node2D

@onready var shield = $Shield
@onready var health_component = $HitComponent

var half_roadsize = (GameManager.roadSize / 2)

var player_left_view = preload("res://assets/sprites/player/player_left_view_lights.png")
var player_right_view = preload("res://assets/sprites/player/player_right_view_lights.png")
var player_front_view = preload("res://assets/sprites/player/no_lights/player_front_view_no_lights.png")


func _ready():
	PlayerVariables.player_node = self
	PlayerVariables.world2d_node = get_parent() # This depends on player being a direct child of MainLevel
	health_component.died.connect(PlayerVariables.defeated) # signal that triggers function when 0 HP
	#PlayerVariables.set_score_timer(true)
	
	
var moveTween: Tween
var stopTween: Tween
var movingDirection : int = 0
var lastMovingDir: int = 1
var movement_speed: float = 0
var shield_move_dir: float = 0

func _physics_process(delta):	
	shield.shouldBounce = true
	movingDirection = 0
	shield_move_dir = 0
	
	if Input.is_action_pressed("move_left"):
		movingDirection -= 1
	if Input.is_action_pressed("move_right"):
		movingDirection += 1
	if Input.is_action_pressed("rotate_shield_right"):
		shield_move_dir += 1		
	if Input.is_action_pressed("rotate_shield_left"):
		shield_move_dir -= 1
		

	movePlayer(delta)	
	changePlayerSprite()
	shield.shield_movement(delta, shield_move_dir)
		
	if shield.shouldBounce:
		shield.shoot_back_bullet()		
	
func changePlayerSprite():
	if movement_speed > 250:
		$SpritePlayer.texture = player_left_view
		$AnimatedSprite2D.visible = false
	elif movement_speed < -250:
		$SpritePlayer.texture = player_right_view
		$AnimatedSprite2D.visible = false
	else:
		$SpritePlayer.texture = player_front_view
		$AnimatedSprite2D.visible = true
		
func movePlayer(delta):
	if movingDirection == 0:
		if moveTween:
			moveTween.kill()
			moveTween = null
		if not stopTween:
			stopTween = create_tween()
			stopTween.tween_property(self, "movement_speed", 0, PlayerVariables.movement_ease_time)
			stopTween.set_ease(Tween.EASE_OUT)
		
	else: 
		if stopTween:
			stopTween.kill()
			stopTween = null
		if not moveTween or lastMovingDir != movingDirection:
			moveTween = create_tween()
			moveTween.tween_property(self, "movement_speed", PlayerVariables.speed * movingDirection, PlayerVariables.movement_ease_time).from(0)
			moveTween.set_ease(Tween.EASE_OUT)
		lastMovingDir = movingDirection
		
	self.position.x += movement_speed * delta
	position.x = clamp(position.x, -half_roadsize, half_roadsize)
	PlayerVariables.player_node3D.position.x = PlayerVariables.IntersectionManager.pos2DGlobalto3DGlobal(PlayerVariables.player_node.global_position).x
