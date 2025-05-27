#include "PlanetMesh.h"
#include "Noise.h"

namespace godot {
    PlanetMesh::PlanetMesh(float radius, int mesh_res, 
        Ref<ShaderMaterial> material, bool mercator,  
        Ref<Texture2D> tile, Vector2 bottom_left_corner_pos, 
        Vector2 top_right_corner_pos)
        : m_radius(radius), m_mesh_res(mesh_res), m_material(material), 
          m_mercator(mercator), m_tile(tile), 
          m_bottom_left_corner_pos(bottom_left_corner_pos), 
          m_top_right_corner_pos(top_right_corner_pos) {
    }
    PlanetMesh::~PlanetMesh(){
    }

    void PlanetMesh::_ready() {
        generate_mesh();
    }

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

        // création d'un mesh carré de côté m_mesh_res
        for (int y = 0; y <= m_mesh_res; ++y) {
            for (int x = 0; x <= m_mesh_res; ++x) {
                Vector2 flat_pos = Vector2(
                    min_x + (max_x - min_x) * ((float)x / m_mesh_res),
                    min_y + (max_y - min_y) * ((float)y / m_mesh_res)
                );

                Vector3 dim3_pos;
                if (m_mercator) {
                    // Mercator projection
                    dim3_pos = Vector3(
                        flat_pos.x * m_radius,
                        flat_pos.y * m_radius,
                        noise(flat_pos) * m_radius
                    );
                } else {
                    float longitude = flat_pos.x * M_PI;
                    float latitude = -(flat_pos.y + 0.5f) * M_PI; 

                    float noise_offset = noise(flat_pos) * 0.1f;
                    float r = (1 + noise_offset) * m_radius;

                    dim3_pos = Vector3(
                        - r * sin(latitude) * cos(longitude),
                        -r * cos(latitude),
                        r * sin(latitude) * sin(longitude)
                );
                }

                vertices.push_back(dim3_pos);

                colors.push_back(Color(1.0f, 1.0f, 1.0f, 1.0f)); // White color for now
                if (m_mercator) {

                    uvs.push_back(Vector2(
                    (float)x / (float)m_mesh_res,  
                    1.0f - (float)y / (float)m_mesh_res
                ));
                } else {
                }
                uvs.push_back(Vector2(
                    (float)x / (float)m_mesh_res,  
                    1.0f - (float)y / (float)m_mesh_res
                ));
            }
        }

        for (int i = 0; i < vertices.size(); ++i) {
            normals.push_back(vertices[i].normalized());
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
        arrays[Mesh::ARRAY_NORMAL] = normals;

        m_array_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);
        set_mesh(m_array_mesh);

        if (m_tile.is_valid()) {
            if (!m_material.is_valid()) {
                m_material.instantiate();
            }
            Ref<ShaderMaterial> mat = m_material->duplicate();
            mat->set_shader_parameter("albedo_texture", m_tile);
            set_surface_override_material(0, mat);
        }
    }

    void PlanetMesh::_bind_methods() {
        
    }
}