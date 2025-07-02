#pragma once

#include <godot_cpp/classes/camera3d.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector3.hpp>

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/json.hpp>
#include <unordered_map>

#include "DebugUI.h"

namespace godot {

struct CityData {
  String name;
  String place_type;
  String culture;
  String religion;
  String population;
  String pol_importance;
  String pol_status;
  std::vector<String> eco_activities;
  std::vector<String> ressources;
  String desc;

  Vector3 color;
};

class CitiesManager : public Node {
  GDCLASS(CitiesManager, Node);

public:
  CitiesManager(Ref<Texture2D> city_texture, DebugUI *debug_ui);
  CitiesManager() = default;
  ~CitiesManager();

  void _ready() override;
  void load_city_mapping();
  void decompress_image();

  void _input(const Ref<InputEvent> &event) override;

private:
  int get_city_id_at_screen_position(Vector2 screen_pos);
  int get_city_id_at_uv_position(Vector2 uv_coords);

  CityData get_city_from_id(int id);
protected:  
  static void _bind_methods();

private:

  Vector3 m_selected_city_color = Vector3(-1.0, -1.0, -1.0);
  int m_selected_city_id = -1;
  std::unordered_map<int, CityData> m_city_mapping;
  Ref<Image> m_cities_image;

  bool m_cities_data_loaded = false;
  
  DebugUI *m_debug_ui = nullptr;
};

} // namespace godot
