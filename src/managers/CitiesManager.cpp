#include "CitiesManager.h"
#include "../Planet.h"
#include "../utils/raycast.h"

namespace godot {
CitiesManager::CitiesManager(Ref<Texture2D> city_texture, DebugUI *debug_ui)
    : m_cities_image(city_texture->get_image()), m_debug_ui(debug_ui) {}

CitiesManager::~CitiesManager() {}

void CitiesManager::_ready() {
  load_city_mapping();
  decompress_image();
}

void CitiesManager::load_city_mapping() {
  if (m_cities_data_loaded) {
    return;
  }
  //
  String json_path = "res://assets/cities/cities.json";
  Ref<FileAccess> file = FileAccess::open(json_path, FileAccess::READ);

  if (!file.is_valid()) {
    UtilityFunctions::print("Failed to load city mapping file: ",
                            json_path);
    return;
  }

  String json_content = file->get_as_text();
  file->close();

  Ref<JSON> json = memnew(JSON);
  Error parse_error = json->parse(json_content);

  if (parse_error != OK) {
    UtilityFunctions::print("Failed to parse JSON: ",
                            json->get_error_message());
    return;
  }

  Dictionary cities = json->get_data();

  // Parcourir toutes les villes
  Array city_keys = cities.keys();
  for (int i = 0; i < city_keys.size(); i++) {
    String key = city_keys[i];
    int city_id = key.to_int();

    Dictionary city_info = cities[key];

    // Vérifier si la villes a des données (pas juste un objet vide)
    if (city_info.has("name")) {
      CityData city_data;
      city_data.name = String(city_info["name"]);
      city_data.place_type = String(city_info.get("place_type", ""));
      // city_data.culture = String(city_info.get("dominant_culture", ""));
      // city_data.religion = String(city_info.get("religion", ""));
      // city_data.population = String(city_info.get("estimated_population", ""));
      // city_data.pol_importance = String(city_info.get("political_importance", ""));
      // city_data.pol_status = String(city_info.get("political_status", ""));
      // Array eco_activities = city_info.get("economic_activities", Array());
      // for (int j = 0; j < eco_activities.size(); j++) {
      //     city_data.eco_activities.push_back(String(eco_activities[j]));
      // }
      // Array resources = city_info.get("local_resources", Array());
      // for (int j = 0; j < resources.size(); j++) {
      //     city_data.ressources.push_back(String(resources[j]));
      // }
      city_data.desc = String(city_info.get("description", ""));

      // Extraire la couleur
      Dictionary color_dict = city_info["color"];
      city_data.color =
          Vector3(float(color_dict["r"]), 
                  float(color_dict["g"]),
                  float(color_dict["b"]));

      m_city_mapping[city_id] = city_data;
    }
  }
  m_cities_data_loaded = true;
  UtilityFunctions::print("Loaded ", int(m_city_mapping.size()),
                          " cities from .json file: ", json_path);
}

  void CitiesManager::decompress_image() {
    if (m_cities_image.is_valid()) {
      // Décompresser l'image si elle est compressée
      if (m_cities_image->is_compressed()) {
        UtilityFunctions::print("Decompressing cities image...");
        m_cities_image->decompress();
      }

      // Vérifier si la décompression a réussi
      if (m_cities_image->is_compressed()) {
        UtilityFunctions::print("Failed to decompress cities image");
        return;
      }

      UtilityFunctions::print("Cities image loaded and decompressed: ",
                              m_cities_image->get_width(), "x",
                              m_cities_image->get_height());
    } else {
        UtilityFunctions::print("Failed to get image from cities texture");
        return;
    }
  }

  void CitiesManager::_input(const Ref<InputEvent> &event) {
    Ref<InputEventMouseButton> mouse_event = event;
    if (mouse_event.is_valid() && mouse_event->is_pressed() &&
        mouse_event->get_button_index() == MOUSE_BUTTON_LEFT) {
      Vector2 mouse_pos = mouse_event->get_position();
      // UtilityFunctions::print("Mouse clicked at: ", mouse_pos.x, ", ",
      //                         mouse_pos.y);
      m_selected_city_id = get_city_id_at_screen_position(mouse_pos);
      // UtilityFunctions::print("City ID at click position: ", city_id);

      if (m_debug_ui == nullptr) {              
        UtilityFunctions::print("Warning: m_debug_ui is null");
        }
      Planet *planet = Object::cast_to<Planet>(get_parent());
      if (planet == nullptr) {
        UtilityFunctions::print("Warning: Planet parent is null");
        return;
      }

      if (m_selected_city_id > 0) { // Si on a trouvé une city

        // Trouver l'ID de la city et ses informations
        CityData city= get_city_from_id(m_selected_city_id);
        if (!city.name.is_empty()) {
          UtilityFunctions::print("Selected city: ", city.name,
                                  " pop: ", city.population,
                                  " culture: ", city.culture,
                                  " religion: ", city.religion);
            m_debug_ui->update_city_name(city.name);

          Vector3 color = Vector3(city.color.x / 255.0f,
                      city.color.y / 255.0f,
                      city.color.z / 255.0f);
          // Clamp les valeurs entre 0 et 1
          color.x = CLAMP(color.x, 0.0f, 1.0f);
          color.y = CLAMP(color.y, 0.0f, 1.0f);
          color.z = CLAMP(color.z, 0.0f, 1.0f);
          
          planet->set_city_to_highlight(color);

        }
      } else {
        UtilityFunctions::print("No city found at click position");
        m_debug_ui->update_city_name("No city found");
      }
    }
  }

  int CitiesManager::get_city_id_at_screen_position(Vector2 screen_pos) {
    Vector2 uv_coords = sphere_raycast(screen_pos, this);
    // UtilityFunctions::print("UV Coordinates: ", uv_coords.x, ", ",
    // uv_coords.y);
    if (uv_coords.x < 0 || uv_coords.x > 1 || uv_coords.y < 0 ||
        uv_coords.y > 1) {
      return 0; // Position invalide
    }

    int province_id = get_city_id_at_uv_position(uv_coords);

    if (province_id > 0 &&
        m_city_mapping.find(province_id) != m_city_mapping.end()) {
      return province_id;
    }

    return 0;


}

int CitiesManager::get_city_id_at_uv_position(Vector2 uv_coords) {
    if (!m_cities_image.is_valid()) {
    UtilityFunctions::print("City image not loaded");
    return 0;
    }   
  
  int img_width = m_cities_image->get_width();
  int img_height = m_cities_image->get_height();
  float value_for_x = uv_coords.x;
  if (value_for_x <= 0.5f) {
    value_for_x += 0.5f;
  } else {
    value_for_x -= 0.5f;
  }
  int pixel_x = int(value_for_x * img_width);

  int pixel_y = int((1.0f - uv_coords.y) * img_height);
  // UtilityFunctions::print("Pixel Coordinates: ", pixel_x, ", ", pixel_y);
  //  S'assurer que les coordonnées sont dans les limites
  pixel_x = CLAMP(pixel_x, 0, img_width - 1);
  pixel_y = CLAMP(pixel_y, 0, img_height - 1);

  // Obtenir la couleur du pixel
  Color pixel_color = m_cities_image->get_pixel(pixel_x, pixel_y);

  // Convertir la couleur en ID de city selon votre format
  // Les couleurs sont stockées comme r, g, b dans le JSON
  int r = int(round(pixel_color.r * 255.0f));
  int g = int(round(pixel_color.g * 255.0f));
  int b = int(round(pixel_color.b * 255.0f));

  //UtilityFunctions::print("Pixel Color: R=", r, ", G=", g, ", B=", b);

  const int color_val = 15;
  int r_index = r / color_val;
  int g_index = g / color_val;
  int b_index = b / color_val;

  int city_id = r_index * (color_val * color_val) + g_index * color_val + b_index;

  return city_id;

}

CityData CitiesManager::get_city_from_id(int city_id) {
  auto it = m_city_mapping.find(city_id);
  if (it != m_city_mapping.end()) {
    return it->second;
  }

  // Retourner une city vide si non trouvée
  return CityData();
}


  void CitiesManager::_bind_methods() {}
} // namespace godot
