#pragma once

#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/material.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <godot_cpp/classes/camera3d.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/viewport.hpp>

#include <godot_cpp/classes/shader.hpp>
#include <godot_cpp/classes/shader_material.hpp>

#include "NCAltitudeReader.h"
#include "PlanetMesh.h"
#include "DebugUI.h"

#include <map>
#include <memory>
#include <vector>

namespace godot {
  class Planet : public Node3D {
    GDCLASS(Planet, Node3D);

  public:
    Planet();
    ~Planet();

    void _ready() override;
    void _process(double delta) override;

    void set_radius(float new_radius);
    float get_radius() const;

    void set_mesh_per_img_res(int res);
    int get_mesh_per_img_res() const;

    void set_mesh_res(int res);
    int get_mesh_res() const;

    void set_material(Ref<ShaderMaterial> material);
    Ref<ShaderMaterial> get_material() const;

    bool get_mercator() const;
    void set_mercator(bool state);

    void generate();
    void generate_colors();

  private:
    void create_textures();
    Camera3D *get_current_camera();
    void generate_visible_meshes();
    void update_visible_meshes();

    void create_submesh_if_needed(int tile_x, int tile_y, int sub_x, int sub_y);
    void remove_submesh_if_exists(int tile_x, int tile_y, int sub_x, int sub_y);
    bool is_submesh_visible(int tile_x, int tile_y, int sub_x, int sub_y, Camera3D* camera);

  private:
    bool m_initialized = false;

    float m_radius = 5.0f;
    Ref<ArrayMesh> m_array_mesh;
    int m_mesh_per_img_res = 2;
    int m_mesh_res = 15;
    Ref<ShaderMaterial> m_material;

    bool m_mercator = false;

    std::vector<PlanetMesh *> m_meshes;

    std::unordered_map<std::string, Ref<Texture2D>> m_tile_cache;
    std::unordered_map<std::string, PlanetMesh *> m_active_meshes;

    std::shared_ptr<NCAltitudeReader> m_elevation_reader;

    DebugUI* m_debug_ui = nullptr;

    Ref<Texture2D> m_country_idx_texture;

  protected:
    static void _bind_methods();
  };

} // namespace godot
