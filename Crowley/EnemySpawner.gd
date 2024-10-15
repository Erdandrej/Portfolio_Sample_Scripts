extends Node

@export_category("Regions")
@export var cityRadius: float 
@export var nestRadius: float 

@export_category("SpawnRates")
@export var initialSpawnsCost: float
@export var spawnCost: float
@export var region3curve: Curve
@export var region2curve: Curve
@export var region1curve: Curve
@export var xToLastCurve: float
var curScore: float

@export_category("Prefabs")
@export var prefabArray: Array[PackedScene]
@export var region1ChanceArray: Array[float]
@export var region2ChanceArray: Array[float]
@export var region3ChanceArray: Array[float]

func _ready():
	IncreaseScore(initialSpawnsCost)
	curScore = 0

func RandomPrioritized(chances: Array):
	var total: float = 0
	for val in chances:
		total += val
	
	var random = randf_range(0.0, total)
	
	var runningTotal: float = 0
	for i in chances.size():
		runningTotal += chances[i]
		if random < runningTotal:
			return i
	# print("random broke")
	return 0
	

func IncreaseScore(newScore):
	var diff = newScore - xToLastCurve

	while curScore < newScore:
		Spawn(curScore)
		curScore += spawnCost
	
	

func Spawn(score: float):
	var t: float = score / xToLastCurve
	
	var r1chance = region1curve.sample(t)
	var r2chance = region1curve.sample(t)
	var r3chance = region1curve.sample(t)
	
	var region = 1 + RandomPrioritized([r1chance, r2chance, r3chance])
	
	var chosenItemChanceArray = null
	match region:
		1: chosenItemChanceArray = region1ChanceArray
		2: chosenItemChanceArray = region2ChanceArray
		3: chosenItemChanceArray = region3ChanceArray
	
	var itemIndex = RandomPrioritized(chosenItemChanceArray)
	
	var item = prefabArray[itemIndex]
	
	SpawnEnemy(item, region)
	

func SpawnEnemy(prefab: PackedScene, region: int):
	var outerRadius = 0
	var innerRadius = 0
	match region:		
		1:
			innerRadius = nestRadius
			outerRadius = cityRadius*0.33
		2:
			innerRadius = cityRadius*0.33
			outerRadius = cityRadius*0.66
		3:
			innerRadius = cityRadius*0.66
			outerRadius = cityRadius
			
	var spawnPointXY = randv_circle(innerRadius, outerRadius)
	
	var spawnPoint = Vector3(spawnPointXY.x, 20, spawnPointXY.y)
	
	var itemObject = prefab.instantiate()
	itemObject.position = spawnPoint
	add_child(itemObject)
	



# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	IncreaseScore(10)
	pass
	
	
func randv_circle(min_radius := 1.0, max_radius := 1.0) -> Vector2:
	var r2_max := max_radius * max_radius
	var r2_min := min_radius * min_radius
	var r := sqrt(randf() * (r2_max - r2_min) + r2_min)
	var t := randf() * TAU
	return Vector2(r, 0).rotated(t)
