#include "PlanetMesh.h"
#include "qffzefEF"

namespace godot {
    PlanetMesh::PlanetMesh(float radius, int mesh_res, 
        Ref<ShaderMaterial> material, bool mercator,  
        Ref<Texture2D> tile, Vector2 bottom_left_corner_pos, 
        Vector2 top_right_corner_pos, std::shared_ptr<NCAltitudeReader> elevation_reader)
        : m_radius(radius), m_mesh_res(mesh_res), m_material(material), 
          m_mercator(mercator), m_tile(tile), 
          m_bottom_left_corner_pos(bottom_left_corner_pos), 
          m_top_right_corner_pos(top_right_corner_pos), m_elevation_reader(elevation_reader){
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
                float x_pos = min_x + (max_x - min_x) * ((float)x / m_mesh_res);
                float y_pos = min_y + (max_y - min_y) * ((float)y / m_mesh_res);
                Vector3 flat_vertex = Vector3(
                    x_pos,
                    y_pos,
                    get_elevation_at_position(Vector2(x_pos, y_pos))
                );

                vertices.push_back(flat_vertex);

                colors.push_back(Color(1.0f, 1.0f, 1.0f, 1.0f)); // White color for now

                uvs.push_back(Vector2(
                    (float)x / (float)m_mesh_res,  
                    1.0f - (float)y / (float)m_mesh_res
                ));

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
        //arrays[Mesh::ARRAY_NORMAL] = normals;

        m_array_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);
        set_mesh(m_array_mesh);

        if (m_tile.is_valid()) {
            if (!m_material.is_valid()) {
                m_material.instantiate();
            }
            Ref<ShaderMaterial> mat = m_material->duplicate();
            mat->set_shader_parameter("albedo_texture", m_tile);
            
            mat->set_shader_parameter("NOCHANGE_planet_radius", m_radius);
            mat->set_shader_parameter("NOCHANGE_use_mercator", m_mercator);
            
            set_surface_override_material(0, mat);
        }
    }

    float PlanetMesh::get_elevation_at_position(const Vector2 &flat_pos) const{
        if (!m_elevation_reader || !m_elevation_reader->is_data_loaded()) {
            //UtilityFunctions::print("No elevation data available.");
            return 0.0f; // Pas de données d'altitude disponibles
        }

        float longitude = flat_pos.x * 180.0f; //de -180 à 180
        float latitude = flat_pos.y * 180.0f; // de -90 à 90

        float elevation = m_elevation_reader->get_elevation_at(latitude, longitude);

        float normalized_elevation = elevation / 8848.0f; // Mont Everest comme référence
    
        return normalized_elevation ; // Facteur d'échelle pour l'effet visuel
    }

    void PlanetMesh::_bind_methods(){
    }
}