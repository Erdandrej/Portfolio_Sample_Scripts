extends Node2D

@export var rotationSpeed: float = 9
@export var easeTime: float = 0.07
## Measured in radians
@export var bounceReflectAngle: float = PI/16

@export var hitbox_area: Area2D

@export_category("Audio")
@export var sfx_absorb: AudioStream
@export var sfx_shoot: AudioStream

var stored_bullets: Array 
var shouldBounce: bool = true

var rng = RandomNumberGenerator.new()
signal absorbed_bullet
signal shot_bullet

var moveTween: Tween
var stopTween: Tween
var rotationSpeed_tweened = 0
var lastMovingDir: int = 1	

func shield_movement(delta, movingDirection: int = 1):
			
	if movingDirection == 0:
		if moveTween:
			moveTween.kill()
			moveTween = null
		if not stopTween:
			stopTween = create_tween()
			stopTween.tween_property(self, "rotationSpeed_tweened", 0, easeTime)
			stopTween.set_ease(Tween.EASE_OUT)
		
	else: 
		shouldBounce = false
		
		if stopTween:
			stopTween.kill()
			stopTween = null
		if not moveTween or lastMovingDir != movingDirection:
			moveTween = create_tween()
			moveTween.tween_property(self, "rotationSpeed_tweened", rotationSpeed * movingDirection, easeTime)
			moveTween.set_ease(Tween.EASE_OUT)
		lastMovingDir = movingDirection
		
	rotation += rotationSpeed_tweened * delta


func _on_attack_area_entered(area):
	var hazard_projectile_instance = area.get_parent() #Might be problematic if area is not direct child of projectile
	if not hazard_projectile_instance is Projectile:
		if area is HitComponent:
			area.instakill()
			return
	stored_bullets.append(hazard_projectile_instance)
	absorbed_bullet.emit()
	hazard_projectile_instance.remove_from_tree()
	AudioManager.play(sfx_absorb)

func wait(seconds: float) -> void:
	await get_tree().create_timer(seconds).timeout

func shoot_back_bullet():
	if stored_bullets.size() > 0:
		for bullet in stored_bullets:
			if bullet != null:
				shoot(bullet)
				
		stored_bullets.clear()


func shoot(bullet: Projectile):
	shot_bullet.emit()
	var bounceRotation = rotation + rng.randf_range(-bounceReflectAngle, bounceReflectAngle)
	var direction = Vector2(cos(bounceRotation), sin(bounceRotation))
	bullet.global_position = PlayerVariables.pos3dglobal_to_2dglobal(global_position) + direction * 120
	bullet.set_target(PlayerVariables.pos2dglobal_to_3dglobal(bullet.global_position) + direction * 50)
	bullet.hit_component.monitoring = false
	bullet.proj3DInstance.get_node("AreaAttack").monitoring = true
	
	bullet.add_to_tree()
	
	bullet.set3DProjectileTransform(true)
	AudioManager.play(sfx_shoot)

