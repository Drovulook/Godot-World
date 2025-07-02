#include "PlanetMesh.h"

namespace godot {
PlanetMesh::PlanetMesh(float radius, int mesh_res, int mesh_per_img_res,
                       Ref<ShaderMaterial> material, bool mercator,
                       Ref<Texture2D> tile, int tile_x, int tile_y, int sub_x,
                       int sub_y, Vector2 bottom_left_corner_pos,
                       Vector2 top_right_corner_pos,
                       std::shared_ptr<NCAltitudeReader> elevation_reader,
                       Ref<Texture2D> province_idx_texture, Ref<Texture2D> city_idx_texture,
                       std::shared_ptr<Vector3> province_to_highlight, std::shared_ptr<Vector3> city_to_highlight)
    : m_radius(radius), m_mesh_res(mesh_res),
      m_mesh_per_img_res(mesh_per_img_res), m_material(material),
      m_mercator(mercator), m_tile(tile), m_tile_x(tile_x), m_tile_y(tile_y),
      m_sub_x(sub_x), m_sub_y(sub_y),
      m_bottom_left_corner_pos(bottom_left_corner_pos),
      m_top_right_corner_pos(top_right_corner_pos),
      m_elevation_reader(elevation_reader),
      m_province_idx_texture(province_idx_texture), m_city_idx_texture(city_idx_texture),
      m_province_to_highlight(province_to_highlight), m_city_to_highlight(city_to_highlight) {}

PlanetMesh::~PlanetMesh() {}

void PlanetMesh::_ready() { generate_mesh(); }

void PlanetMesh::generate_mesh() {
  m_array_mesh.instantiate();

  PackedVector3Array vertices;
  PackedInt32Array indices;
  PackedVector2Array uvs;
  PackedColorArray colors;
  PackedVector3Array normals;

  float min_x = m_bottom_left_corner_pos.x;
  float min_y = m_bottom_left_corner_pos.y;
  float max_x = m_top_right_corner_pos.x;
  float max_y = m_top_right_corner_pos.y;

  float uv_dim = 1.0f / m_mesh_per_img_res;
  float uv_offset_x = m_sub_x * uv_dim;
  float uv_offset_y = (m_mesh_per_img_res - 1 - m_sub_y) * uv_dim;

  // création d'un mesh carré de côté m_mesh_res
  for (int y = 0; y <= m_mesh_res; ++y) {
    for (int x = 0; x <= m_mesh_res; ++x) {
      float x_pos = min_x + (max_x - min_x) * ((float)x / m_mesh_res);
      float y_pos = min_y + (max_y - min_y) * ((float)y / m_mesh_res);
      Vector3 flat_vertex = Vector3(x_pos, y_pos, 0.0f);

      vertices.push_back(flat_vertex);

      colors.push_back(Color(1.0f, 1.0f, 1.0f, 1.0f)); // White color for now

      float local_u = (float)x / (float)m_mesh_res;
      float local_v = 1.0f - (float)y / (float)m_mesh_res;
      uvs.push_back(Vector2(uv_offset_x + local_u * uv_dim,
                            uv_offset_y + local_v * uv_dim));
    }
  }

  for (int y = 0; y < m_mesh_res; ++y) {
    for (int x = 0; x < m_mesh_res; ++x) {
      int i = y * (m_mesh_res + 1) + x;

      if (m_mercator) {
        indices.push_back(i);
        indices.push_back(i + m_mesh_res + 1);
        indices.push_back(i + 1);

        indices.push_back(i + 1);
        indices.push_back(i + m_mesh_res + 1);
        indices.push_back(i + m_mesh_res + 2);
      } else {
        indices.push_back(i);
        indices.push_back(i + m_mesh_res + 1);
        indices.push_back(i + 1);

        indices.push_back(i + 1);
        indices.push_back(i + m_mesh_res + 1);
        indices.push_back(i + m_mesh_res + 2);
      }
    }
  }

  Array arrays;
  arrays.resize(Mesh::ARRAY_MAX);
  arrays[Mesh::ARRAY_VERTEX] = vertices;
  arrays[Mesh::ARRAY_INDEX] = indices;
  arrays[Mesh::ARRAY_TEX_UV] = uvs;
  arrays[Mesh::ARRAY_COLOR] = colors;
  // arrays[Mesh::ARRAY_NORMAL] = normals;

  m_array_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);
  set_mesh(m_array_mesh);

  if (m_tile.is_valid()) {
    if (!m_material.is_valid()) {
      m_material.instantiate();
    }
    Ref<ShaderMaterial> mat = m_material->duplicate();
    mat->set_shader_parameter("albedo_texture", m_tile);
    mat->set_shader_parameter("province_idx_texture", m_province_idx_texture);
    mat->set_shader_parameter("city_idx_texture", m_city_idx_texture);

    if (m_elevation_reader && m_elevation_reader->is_data_loaded()) {
      Ref<ImageTexture> elevation_tex =
          m_elevation_reader->get_elevation_texture();
      if (elevation_tex.is_valid()) {
        mat->set_shader_parameter("elevation_texture", elevation_tex);
        mat->set_shader_parameter("NOCHANGE_min_alt",
                                  m_elevation_reader->get_min_elevation());
        mat->set_shader_parameter("NOCHANGE_max_alt",
                                  m_elevation_reader->get_max_elevation());
        mat->set_shader_parameter("NOCHANGE_tile_x", (float)m_tile_x);
        mat->set_shader_parameter("NOCHANGE_tile_y", (float)m_tile_y);
        mat->set_shader_parameter("NOCHANGE_sub_x", (float)m_sub_x);
        mat->set_shader_parameter("NOCHANGE_sub_y", (float)m_sub_y);
        mat->set_shader_parameter("NOCHANGE_mesh_res", (float)m_mesh_res);
        mat->set_shader_parameter("NOCHANGE_img_res_in_mesh",
                                  (float)m_mesh_per_img_res);
        // UtilityFunctions::print(*m_province_to_highlight);
        mat->set_shader_parameter("NOCHANGE_province_color_to_highlight", *m_province_to_highlight);
        mat->set_shader_parameter("NOCHANGE_city_color_to_highlight", *m_city_to_highlight);
      } else {
        UtilityFunctions::print("Elevation texture is not valid. Please check the NCAltitudeReader setup.");
      }
    }

    mat->set_shader_parameter("NOCHANGE_planet_radius", m_radius);
    mat->set_shader_parameter("NOCHANGE_use_mercator", m_mercator);

    set_surface_override_material(0, mat);
  }
}

void PlanetMesh::_bind_methods() {}
} // namespace godot
