#include "Manager.h"
#include "../Planet.h"

namespace godot {

Manager::Manager(const Ref<Texture2D> &texture, DebugUI *debug_ui, String path)
    : m_texture(texture), m_debug_ui(debug_ui), m_path(path) {}

Manager::~Manager() {}

void Manager::_ready() {
  load_mapping();
  decompress_image();
}

void Manager::decompress_image() {
  m_obj_image = m_texture->get_image()->duplicate();
  if (m_obj_image.is_valid()) {
    // Décompresser l'image si elle est compressée
    if (m_obj_image->is_compressed()) {
      UtilityFunctions::print("Decompressing city
 image...");
      m_obj_image->decompress();
    }

    // Vérifier si la décompression a réussi
    if (m_obj_image->is_compressed()) {
      UtilityFunctions::print("Failed to decompress city
 image");
      return;
    }

    UtilityFunctions::print(
        "Province image loaded and decompressed: ", m_obj_image->get_width(),
        "x", m_obj_image->get_height());
  } else {
    UtilityFunctions::print("Failed to get image from city
 texture");
    return;
  }
}

void Manager::_input(const Ref<InputEvent> &event) {
  Ref<InputEventMouseButton> mouse_event = event;
  if (mouse_event.is_valid() && mouse_event->is_pressed() &&
      mouse_event->get_button_index() == MOUSE_BUTTON_LEFT) {
    Vector2 mouse_pos = mouse_event->get_position();
    // UtilityFunctions::print("Mouse clicked at: ", mouse_pos.x, ", ",
    //                         mouse_pos.y);
    int city
_id = get_obj_at_screen_position(mouse_pos);
    // UtilityFunctions::print("Country ID at click position: ", city
_id);
    m_selected_obj_id = city
_id;

    if (city
_id >= 0) { // Si on a trouvé une city

      // Trouver l'ID de la city
 et ses informations
      ManagerData data = get_obj_from_id(m_selected_obj_id);
      if (!data.name.is_empty()) {
        UtilityFunctions::print("Selected city
: ", data.name, " in ",
                                data.name, " (", data.code, ")");
        m_debug_ui->update_city
_name(data.name);
        Planet *planet = Object::cast_to<Planet>(get_parent());
        planet->set_city
_to_highlight(Vector3(data.color.x / 255.0f,
                                                  data.color.y / 255.0f,
                                                  data.color.z / 255.0f));
      }
    } else {
      UtilityFunctions::print("No city
 found at click position");
    }
  }
}

int Manager::get_obj_at_screen_position(Vector2 screen_pos) { return 0; }

Vector2 Manager::screen_to_uv_coordinates(Vector2 screen_pos) {}

int Manager::get_obj_id_at_position(Vector2 uv_coords) {}

ManagerData Manager::get_obj_from_id(int id) {}

static void _bind_methods() {}

} // namespace godot
