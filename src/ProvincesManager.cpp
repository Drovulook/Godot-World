#include "ProvincesManager.h"
#include "Planet.h"

namespace godot {

ProvincesManager::ProvincesManager(Ref<Texture2D> province_texture,
                                   DebugUI *debug_ui)
    : m_province_image(province_texture->get_image()), m_debug_ui(debug_ui) {}

ProvincesManager::~ProvincesManager() {}

void ProvincesManager::_ready() {
  load_province_mapping();
  decompress_image();
}

void ProvincesManager::load_province_mapping() {
  if (m_province_data_loaded) {
    return;
  }
  //
  String json_path =
      "res://assets/provinces/Province_Index_Map_Generated_mapping.json";
  Ref<FileAccess> file = FileAccess::open(json_path, FileAccess::READ);

  if (!file.is_valid()) {
    UtilityFunctions::print("Failed to load province mapping file: ",
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

  Dictionary data = json->get_data();
  Dictionary provinces = data["provinces"];

  // Parcourir toutes les provinces
  Array province_keys = provinces.keys();
  for (int i = 0; i < province_keys.size(); i++) {
    String key = province_keys[i];
    int province_id = key.to_int();

    Dictionary province_info = provinces[key];

    // Vérifier si la province a des données (pas juste un objet vide)
    if (province_info.has("country_code")) {
      ProvinceData province_data;
      province_data.country_code = String(province_info["country_code"]);
      province_data.country_name = String(province_info["country_name"]);
      province_data.province_name = String(province_info["province_name"]);
      province_data.province_type = String(province_info.get("province_type", ""));

      // Extraire la couleur
      Dictionary color_dict = province_info["color"];
      province_data.color =
          Vector3(float(color_dict["r"]), float(color_dict["g"]),
                  float(color_dict["b"]));

      m_province_mapping[province_id] = province_data;
    }
  }

  m_province_data_loaded = true;
  UtilityFunctions::print("Loaded ", int(m_province_mapping.size()),
                          " provinces from .json file: ", json_path);
}

void ProvincesManager::decompress_image() {
  if (m_province_image.is_valid()) {
    // Décompresser l'image si elle est compressée
    if (m_province_image->is_compressed()) {
      UtilityFunctions::print("Decompressing province image...");
      m_province_image->decompress();
    }

    // Vérifier si la décompression a réussi
    if (m_province_image->is_compressed()) {
      UtilityFunctions::print("Failed to decompress province image");
      return;
    }

    UtilityFunctions::print("Province image loaded and decompressed: ",
                            m_province_image->get_width(), "x",
                            m_province_image->get_height());
  } else {
    UtilityFunctions::print("Failed to get image from province texture");
    return;
  }
}

void ProvincesManager::_input(const Ref<InputEvent> &event) {
  Ref<InputEventMouseButton> mouse_event = event;
  if (mouse_event.is_valid() && mouse_event->is_pressed() &&
      mouse_event->get_button_index() == MOUSE_BUTTON_LEFT) {
    Vector2 mouse_pos = mouse_event->get_position();
    // UtilityFunctions::print("Mouse clicked at: ", mouse_pos.x, ", ",
    //                         mouse_pos.y);
    int province_id = get_country_at_screen_position(mouse_pos);
    // UtilityFunctions::print("Country ID at click position: ", province_id);
    m_selected_province_id = province_id;

    if (province_id >= 0) { // Si on a trouvé une province

      // Trouver l'ID de la province et ses informations
      ProvinceData province = get_province_from_id(m_selected_province_id);
      if (!province.province_name.is_empty()) {
        UtilityFunctions::print(
            "Selected province: ", province.province_name, " in ",
            province.country_name, " (", province.country_code,
            ")");
        m_debug_ui->update_province_name(province.province_name);
        Planet *planet = Object::cast_to<Planet>(get_parent());
        planet->set_province_to_highlight(Vector3(province.color.x / 255.0f,
                                                  province.color.y / 255.0f,
                                                  province.color.z / 255.0f));
      }
    } else {
      UtilityFunctions::print("No province found at click position");
    }
  }
}

int ProvincesManager::get_country_at_screen_position(Vector2 screen_pos) {
  Vector2 uv_coords = screen_to_uv_coordinates(screen_pos);
  // UtilityFunctions::print("UV Coordinates: ", uv_coords.x, ", ",
  // uv_coords.y);
  if (uv_coords.x < 0 || uv_coords.x > 1 || uv_coords.y < 0 ||
      uv_coords.y > 1) {
    return 0; // Position invalide
  }

  // Obtenir l'ID de la province à cette position UV
  int province_id = get_country_id_at_position(uv_coords);

  if (province_id > 0 &&
      m_province_mapping.find(province_id) != m_province_mapping.end()) {
    return province_id;
    ;
  }

  return 0;
}

Vector2 ProvincesManager::screen_to_uv_coordinates(Vector2 screen_pos) {
  Viewport *viewport = get_viewport();
  if (!viewport) {
    UtilityFunctions::print("No viewport found");
    return Vector2(-1, -1);
  }

  Camera3D *camera = viewport->get_camera_3d();
  if (!camera) {
    UtilityFunctions::print("No camera found");
    return Vector2(-1, -1);
  }

  // Obtenir la planète (parent de ce manager)
  Planet *planet = Object::cast_to<Planet>(get_parent());
  if (!planet) {
    UtilityFunctions::print("No planet found as parent");
    return Vector2(-1, -1);
  }

  Vector3 ray_origin = camera->project_ray_origin(screen_pos);
  Vector3 ray_direction = camera->project_ray_normal(screen_pos);

  // Position et rayon de la planète
  Vector3 planet_center = planet->get_global_transform().origin;
  float planet_radius =
      planet->get_radius(); // Utiliser la valeur par défaut du radius

  // Intersection rayon-sphère
  Vector3 to_center = ray_origin - planet_center;
  float a = ray_direction.dot(ray_direction);
  float b = 2.0f * to_center.dot(ray_direction);
  float c = to_center.dot(to_center) - planet_radius * planet_radius;

  float discriminant = b * b - 4 * a * c;

  if (discriminant < 0) {
    return Vector2(-1, -1); // Pas d'intersection
  }

  // Prendre l'intersection la plus proche
  float t = (-b - sqrt(discriminant)) / (2 * a);
  if (t < 0) {
    t = (-b + sqrt(discriminant)) / (2 * a);
  }

  if (t < 0) {
    return Vector2(-1, -1);
  }

  // Point d'intersection sur la sphère
  Vector3 hit_point = ray_origin + ray_direction * t;

  // Convertir en coordonnées locales de la planète
  Vector3 local_hit =
      planet->get_global_transform().affine_inverse().xform(hit_point);

  // Convertir en coordonnées UV (projection équirectangulaire)
  Vector3 normalized = local_hit.normalized();

  // Calculer longitude et latitude
  float longitude = atan2(normalized.z, -normalized.x);
  float latitude = asin(-normalized.y);

  // Convertir en coordonnées UV (0-1)
  Vector2 uv_coords;
  uv_coords.x = (longitude / M_PI + 1.0f) * 0.5f;
  uv_coords.y = (-latitude / M_PI + 0.5f);

  return uv_coords;
}

int ProvincesManager::get_country_id_at_position(Vector2 uv_coords) {
  if (!m_province_image.is_valid()) {
    UtilityFunctions::print("Province image not loaded");
    return 0;
  }

  // Convertir les coordonnées UV en coordonnées pixel
  int img_width = m_province_image->get_width();
  int img_height = m_province_image->get_height();
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
  Color pixel_color = m_province_image->get_pixel(pixel_x, pixel_y);

  // Convertir la couleur en ID de province selon votre format
  // Les couleurs sont stockées comme r, g, b dans le JSON
  int r = int(round(pixel_color.r * 255.0f));
  int g = int(round(pixel_color.g * 255.0f));
  int b = int(round(pixel_color.b * 255.0f));
  
  UtilityFunctions::print("Pixel Color: R=", r, ", G=", g, ", B=", b);
  
  // CORRECTION : Utiliser le même encodage que Python (bitshift)
  int province_id = (r << 16) | (g << 8) | b;

  // Si cette approche ne fonctionne pas, essayez cette alternative :
  // int province_id = b; // Si l'ID est juste dans le canal bleu

  return province_id;
}

ProvinceData ProvincesManager::get_province_from_id(int province_id) {
  auto it = m_province_mapping.find(province_id);
  if (it != m_province_mapping.end()) {
    return it->second;
  }

  // Retourner une province vide si non trouvée
  return ProvinceData();
}

void ProvincesManager::_bind_methods() {}

} // namespace godot
