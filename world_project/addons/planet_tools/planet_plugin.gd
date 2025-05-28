@tool
extends EditorPlugin

const PlanetInspectorPlugin = preload("res://addons/planet_tools/planet_inspector_plugin.gd")
var inspector_plugin

func _enter_tree():
	inspector_plugin = PlanetInspectorPlugin.new()
	add_inspector_plugin(inspector_plugin)
	print("plugin created")

func _exit_tree():
	remove_inspector_plugin(inspector_plugin)
