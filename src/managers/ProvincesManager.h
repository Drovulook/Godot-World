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

struct ProvinceData {
  String country_code;
  String country_name;
  String province_name;
  String province_type;
  Vector3 color;
};

class ProvincesManager : public Node {
  GDCLASS(ProvincesManager, Node);

public:
  ProvincesManager(Ref<Texture2D> province_texture, DebugUI *debug_ui);

  ProvincesManager() = default;
  ~ProvincesManager();

  void _ready() override;
  void load_province_mapping();
  void decompress_image();

  void _input(const Ref<InputEvent> &event) override;

  String get_selected_province_name() const;
  String get_selected_country_name() const;
  String get_selected_country_code() const;
  int get_selected_province_id() const;

private:
  int get_country_id_at_screen_position(Vector2 screen_pos);
  int get_country_id_at_uv_position(Vector2 uv_coords);

  ProvinceData get_province_from_id(int id);

protected:
  static void _bind_methods();

private:
  Vector3 m_selected_country_color = Vector3(-1.0, -1.0, -1.0);
  int m_selected_province_id = -1;

  std::unordered_map<int, ProvinceData> m_province_mapping;

  Ref<Image> m_province_image;
  
  bool m_province_data_loaded = false;

  DebugUI *m_debug_ui = nullptr;
};

} // namespace godot
