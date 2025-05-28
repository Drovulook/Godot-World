@tool
extends EditorInspectorPlugin

func _can_handle(object):
	# Vérifier si l'objet est une instance de Planet
	# Debug pour voir ce qui se passe
	if object == null:
		return false
	
	# Pour une classe C++, vérifier le nom de classe exact
	var className = object.get_class()
	
	# Votre classe C++ devrait apparaître comme "Planet"
	if className == "Planet":
		print("✅ Found Planet class for plugin!")
		return true

func _parse_begin(object):
	# Ajouter le bouton au début de l'inspecteur
	var button = Button.new()
	button.text = "🔄 Generate Planet"
	button.custom_minimum_size = Vector2(0, 30)
	
	# Connecter le signal du bouton
	button.pressed.connect(_on_generate_pressed.bind(object))
	
	# Ajouter le bouton à l'inspecteur
	add_custom_control(button)

func _on_generate_pressed(planet_object):
	print("Generating planet...")
	if planet_object and planet_object.has_method("generate"):
		planet_object.generate()
	else:
		print("Error: Invalid planet object!")
