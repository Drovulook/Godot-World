#include "Planet.h"
#include <godot_cpp/classes/camera3d.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "maths_func.h"

namespace godot {
Planet::Planet() {}

Planet::~Planet() {}

void Planet::_ready() {

  UtilityFunctions::print("Planet::_ready() - Initializing NCReader");
  String godot_path = "res://assets/earth_data.nc";
  String abs_path =
      ProjectSettings::get_singleton()->globalize_path(godot_path);
  std::string file_path = abs_path.utf8().get_data();
  m_elevation_reader = std::make_shared<NCAltitudeReader>(file_path);
  if (!m_elevation_reader->load_file()) {
    UtilityFunctions::print("Failed to load elevation data!");
    m_elevation_reader = nullptr;
  } else {
    UtilityFunctions::print("Elevation data loaded successfully!");
  }
  m_elevation_reader->create_elevation_texture();
  UtilityFunctions::print("texture created");

  if (!Engine::get_singleton()->is_editor_hint()) {
    m_debug_ui = memnew(DebugUI);
    add_child(m_debug_ui);
  }

  create_textures();
  m_provinces_manager =
      memnew(ProvincesManager(m_province_idx_texture, m_debug_ui));
  add_child(m_provinces_manager);
  m_cities_manager = memnew(CitiesManager(m_city_idx_texture, m_debug_ui));
  add_child(m_cities_manager);

  // Précalculer les infos de tiles
  m_tile_corner_cache.resize(32); // 8x4 tiles
  for (int x = 0; x < 8; ++x) {
    for (int y = 0; y < 4; ++y) {
      int idx = y * 8 + x;
      m_tile_corner_cache[idx].bottom_left =
          Vector2(2 * (x / 8.0f - 0.5f), y / 4.0f - 0.5f);
      m_tile_corner_cache[idx].top_right =
          Vector2(2 * ((x + 1) / 8.0f - 0.5f), (y + 1) / 4.0f - 0.5f);
      m_tile_corner_cache[idx].sub_width =
          (m_tile_corner_cache[idx].top_right.x -
           m_tile_corner_cache[idx].bottom_left.x) /
          m_mesh_per_img_res;
      m_tile_corner_cache[idx].sub_height =
          (m_tile_corner_cache[idx].top_right.y -
           m_tile_corner_cache[idx].bottom_left.y) /
          m_mesh_per_img_res;
    }
  }

  set_process(true);
  generate_visible_meshes();

  m_initialized = true;
}

void Planet::_process(double delta) {
  static int frame_counter = 0;
  static Vector3 last_camera_pos = Vector3();
  static float accumulated_delta = 0.0f;

  accumulated_delta += delta;

  // OPTIMISATION: Update seulement quand nécessaire
  Camera3D *camera = get_current_camera();
  if (camera) {
    Vector3 current_pos = camera->get_global_transform().origin;
    float movement = current_pos.distance_to(last_camera_pos);

    // Update seulement si la caméra a bougé significativement OU toutes les 0.1
    // secondes
    if (movement > m_radius * 0.01f || accumulated_delta > 0.1f) {
      update_visible_meshes();
      last_camera_pos = current_pos;
      accumulated_delta = 0.0f;

      if (m_debug_ui && !Engine::get_singleton()->is_editor_hint()) {
        m_debug_ui->update_mesh_count(m_active_meshes.size());
      }
    }
  }
}

void Planet::set_radius(float new_radius) {
  m_radius = new_radius;
  if (m_initialized)
    generate();
}

float Planet::get_radius() const { return m_radius; }

void Planet::set_mesh_per_img_res(int res) {
  m_mesh_per_img_res = res;
  if (m_initialized)
    generate();
}

int Planet::get_mesh_per_img_res() const { return m_mesh_per_img_res; }

void Planet::set_mesh_res(int res) {
  m_mesh_res = res;
  if (m_initialized)
    generate();
}

int Planet::get_mesh_res() const { return m_mesh_res; }

bool Planet::get_mercator() const { return m_mercator; }

void Planet::set_mercator(bool state) {
  m_mercator = state;
  if (m_initialized)
    generate();
}

void Planet::set_material(Ref<ShaderMaterial> material) {
  m_material = material;

  if (m_material.is_valid() && m_initialized) {
    generate_visible_meshes();
  }
}

Ref<ShaderMaterial> Planet::get_material() const { return m_material; }

void Planet::generate() {

  for (auto &pair : m_active_meshes) {
    pair.second->queue_free();
  }
  m_active_meshes.clear();

  generate_visible_meshes();
}

void Planet::generate_colors() {}

void Planet::set_province_to_highlight(Vector3 color) {
  m_province_to_highlight = std::make_shared<Vector3>(color);
  generate();
}

void Planet::create_textures() {
  // Create a texture map for the planet
  for (int x = 0; x < 8; ++x) {
    for (int y = 0; y < 4; ++y) {
      std::string tile_name = std::to_string(x) + "_" + std::to_string(y);
      std::string tile_path = "res://assets/tiles/" + tile_name + ".jpg";
      Ref<Texture2D> tile_texture =
          ResourceLoader::get_singleton()->load(tile_path.c_str());
      if (tile_texture.is_valid()) {
        m_tile_cache[tile_name] = tile_texture;
      }
    }
  }

  m_province_idx_texture = ResourceLoader::get_singleton()->load(
      "res://assets/provinces/Province_Index_Map_Generated.png");
  m_city_idx_texture = ResourceLoader::get_singleton()->load(
      "res://assets/cities/cities.png");
}

Camera3D *Planet::get_current_camera() {
  // Obtenir la caméra actuelle (éditeur ou jeu)
  if (Engine::get_singleton()->is_editor_hint()) {
    // En mode éditeur, chercher la caméra de l'éditeur
    return nullptr; // L'éditeur nécessite une approche différente
  } else {
    // En mode jeu
    Viewport *viewport = get_viewport();
    if (viewport) {
      Camera3D *camera = viewport->get_camera_3d();
      return camera;
    }
  }
  return nullptr;
}

void Planet::generate_visible_meshes() {
  Camera3D *camera = get_current_camera();

  std::vector<std::string> to_remove;
  std::vector<std::tuple<int, int, int, int>> to_create;

  for (int x = 0; x < 8; x++) {
    for (int y = 0; y < 4; y++) {
      for (int sub_x = 0; sub_x < m_mesh_per_img_res; sub_x++) {
        for (int sub_y = 0; sub_y < m_mesh_per_img_res; sub_y++) {
          std::string mesh_id = std::to_string(x) + "_" + std::to_string(y) +
                                "_" + std::to_string(sub_x) + "_" +
                                std::to_string(sub_y);

          bool is_visible = is_submesh_visible(x, y, sub_x, sub_y, camera);
          bool exists = m_active_meshes.find(mesh_id) != m_active_meshes.end();

          if (is_visible && !exists) {
            to_create.push_back({x, y, sub_x, sub_y});
          } else if (!is_visible && exists) {
            to_remove.push_back(mesh_id);
          }
        }
      }
    }
  }

  // Exécuter les opérations par batch
  for (const auto &mesh_id : to_remove) {
    auto it = m_active_meshes.find(mesh_id);
    if (it != m_active_meshes.end()) {
      it->second->queue_free();
      m_active_meshes.erase(it);
    }
  }

  for (const auto &[x, y, sub_x, sub_y] : to_create) {
    create_submesh_if_needed(x, y, sub_x, sub_y);
  }
}

void Planet::update_visible_meshes() { generate_visible_meshes(); }

void Planet::create_submesh_if_needed(int tile_x, int tile_y, int sub_x,
                                      int sub_y) {
  std::string mesh_id = std::to_string(tile_x) + "_" + std::to_string(tile_y) +
                        "_" + std::to_string(sub_x) + "_" +
                        std::to_string(sub_y);

  // Vérifier si le mesh existe déjà
  auto mesh_it = m_active_meshes.find(mesh_id);
  if (mesh_it != m_active_meshes.end()) {
    return; // Mesh déjà créé
  }

  if (!m_material.is_valid()) {
    return;
  }

  Vector2 tile_bottom_left =
      Vector2(2 * (tile_x / 8.0f - 0.5f), tile_y / 4.0f - 0.5f);
  Vector2 tile_top_right =
      Vector2(2 * ((tile_x + 1) / 8.0f - 0.5f), (tile_y + 1) / 4.0f - 0.5f);

  // Calculer les dimensions du sous-mesh
  float sub_width =
      (tile_top_right.x - tile_bottom_left.x) / m_mesh_per_img_res;
  float sub_height =
      (tile_top_right.y - tile_bottom_left.y) / m_mesh_per_img_res;

  Vector2 submesh_bottom_left =
      Vector2(tile_bottom_left.x + sub_x * sub_width,
              tile_bottom_left.y + sub_y * sub_height);
  Vector2 submesh_top_right =
      Vector2(tile_bottom_left.x + (sub_x + 1) * sub_width,
              tile_bottom_left.y + (sub_y + 1) * sub_height);

  std::string tile_id = std::to_string(tile_x) + "_" + std::to_string(tile_y);
  Ref<Texture2D> tile;
  auto texture_it = m_tile_cache.find(tile_id);
  if (texture_it != m_tile_cache.end()) {
    tile = texture_it->second;
  }

  PlanetMesh *mesh = memnew(PlanetMesh(
      m_radius, m_mesh_res, m_mesh_per_img_res, m_material, m_mercator, tile,
      tile_x, tile_y, sub_x, sub_y, submesh_bottom_left, submesh_top_right,
      m_elevation_reader, m_province_idx_texture, m_city_idx_texture, m_province_to_highlight));
  add_child(mesh);
  m_active_meshes[mesh_id] = mesh;
}

bool Planet::is_submesh_visible(int tile_x, int tile_y, int sub_x, int sub_y,
                                Camera3D *camera) {
  if (!camera)
    return true;

  // Calculer les positions du tile principal
  Vector2 tile_bottom_left =
      Vector2(2 * (tile_x / 8.0f - 0.5f), tile_y / 4.0f - 0.5f);
  Vector2 tile_top_right =
      Vector2(2 * ((tile_x + 1) / 8.0f - 0.5f), (tile_y + 1) / 4.0f - 0.5f);

  // Calculer les dimensions du sous-mesh
  float sub_width =
      (tile_top_right.x - tile_bottom_left.x) / m_mesh_per_img_res;
  float sub_height =
      (tile_top_right.y - tile_bottom_left.y) / m_mesh_per_img_res;

  // Calculer le centre du sous-mesh
  int tile_idx = tile_y * 8 + tile_x;
  const TileInfo &tile = m_tile_corner_cache[tile_idx];

  Vector2 submesh_center =
      Vector2(tile.bottom_left.x + (sub_x + 0.5f) * tile.sub_width,
              tile.bottom_left.y + (sub_y + 0.5f) * tile.sub_height);

  Vector3 world_center;
  if (m_mercator) {
    world_center =
        Vector3(submesh_center.x * m_radius, submesh_center.y * m_radius, 0);
  } else {
    world_center = get_spherical_pos_from_flat_pos(submesh_center, m_radius);
  }

  world_center = get_global_transform().xform(world_center);

  Vector3 cam_pos = camera->get_global_transform().origin;
  float dist = world_center.distance_to(cam_pos);

  if (dist > m_radius * 1.0f)
    return false;

  if (camera->is_position_in_frustum(world_center) || dist < m_radius * 0.15f)
    return true;

  Vector2 corners[4] = {
      Vector2(tile_bottom_left.x + sub_x * sub_width,
              tile_bottom_left.y + sub_y * sub_height), // bottom-left
      Vector2(tile_bottom_left.x + (sub_x + 1) * sub_width,
              tile_bottom_left.y + sub_y * sub_height), // bottom-right
      Vector2(tile_bottom_left.x + (sub_x + 1) * sub_width,
              tile_bottom_left.y + (sub_y + 1) * sub_height), // top-right
      Vector2(tile_bottom_left.x + sub_x * sub_width,
              tile_bottom_left.y + (sub_y + 1) * sub_height) // top-left
  };

  // Test des quatre sommets
  for (int i = 0; i < 4; i++) {
    Vector3 world_corner;
    if (m_mercator) {
      world_corner =
          Vector3(corners[i].x * m_radius, corners[i].y * m_radius, 0);
    } else {
      world_corner = get_spherical_pos_from_flat_pos(corners[i], m_radius);
    }

    world_corner = get_global_transform().xform(world_corner);

    if (camera->is_position_in_frustum(world_corner)) {
      return true;
    }
  }

  // Si aucun point n'est visible, le mesh est invisible
  return false;
}

void Planet::_bind_methods() {
  ClassDB::bind_method(D_METHOD("set_radius", "new_radius"),
                       &Planet::set_radius);
  ClassDB::bind_method(D_METHOD("get_radius"), &Planet::get_radius);
  ClassDB::bind_method(D_METHOD("set_mesh_per_img_res", "res"),
                       &Planet::set_mesh_per_img_res);
  ClassDB::bind_method(D_METHOD("get_mesh_per_img_res"),
                       &Planet::get_mesh_per_img_res);
  ClassDB::bind_method(D_METHOD("set_mesh_res", "res"), &Planet::set_mesh_res);
  ClassDB::bind_method(D_METHOD("get_mesh_res"), &Planet::get_mesh_res);
  ClassDB::bind_method(D_METHOD("set_material", "m_material"),
                       &Planet::set_material);
  ClassDB::bind_method(D_METHOD("get_material"), &Planet::get_material);
  ClassDB::bind_method(D_METHOD("get_mercator"), &Planet::get_mercator);
  ClassDB::bind_method(D_METHOD("set_mercator", "state"),
                       &Planet::set_mercator);

  ClassDB::bind_method(D_METHOD("generate"), &Planet::generate);

  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "radius"), "set_radius",
               "get_radius");
  ADD_PROPERTY(PropertyInfo(Variant::INT, "mesh_per_img_res",
                            PROPERTY_HINT_RANGE, "1,50,1"),
               "set_mesh_per_img_res", "get_mesh_per_img_res");
  ADD_PROPERTY(
      PropertyInfo(Variant::INT, "mesh_res", PROPERTY_HINT_RANGE, "1,800,1"),
      "set_mesh_res", "get_mesh_res");
  ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "m_material",
                            PROPERTY_HINT_RESOURCE_TYPE, "ShaderMaterial"),
               "set_material", "get_material");
  ADD_PROPERTY(PropertyInfo(Variant::BOOL, "m_mercator"), "set_mercator",
               "get_mercator");
}
} // namespace godot
