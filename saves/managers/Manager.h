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

struct ManagerData {
  String code;
  String name;
  String province_name;
  String province_type;
  Vector3 color;
};

class Manager : public Node {
  GDCLASS(Manager, Node);

public:
  Manager(const Ref<Texture2D> &texture, DebugUI *debug_ui, String path);

  Manager() = default;
  ~Manager();

  void _ready() override;

  virtual void load_mapping() = 0;
  void decompress_image();

  void _input(const Ref<InputEvent> &event) override;

private:
  int get_obj_at_screen_position(Vector2 screen_pos);
  Vector2 screen_to_uv_coordinates(Vector2 screen_pos);
  int get_obj_id_at_position(Vector2 uv_coords);

  ManagerData get_obj_from_id(int id);

protected:
  static void _bind_methods();

private:
  Vector3 m_selected_obj_color = Vector3(-1.0, -1.0, -1.0);
  int m_selected_obj_id = -1;

  String m_path;

  std::unordered_map<int, ManagerData> m_obj_mapping;

  Ref<Texture2D> m_texture;
  Ref<Image> m_obj_image;
  bool m_obj_data_loaded = false;

  DebugUI *m_debug_ui = nullptr;
};

} // namespace godot
