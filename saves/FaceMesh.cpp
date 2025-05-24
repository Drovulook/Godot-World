
#include <map>
#include <string>

#include "FaceMesh.h"
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/variant/packed_vector2_array.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/classes/texture2d.hpp>

#include "Noise.h"
#include "maths_func.h"

namespace godot {

    std::map<std::string, Ref<Texture2D>> FaceMesh::tile_cache;

    FaceMesh::FaceMesh(FaceDirection direction, float radius, int mesh_res, 
        int mesh_per_face_res, int ind_i, int ind_j, Ref<StandardMaterial3D> material, bool merc_state) 
        : m_direction(direction), m_radius(radius), m_mesh_res(mesh_res), 
        m_mesh_per_face_res(mesh_per_face_res), m_ind_i(ind_i), m_ind_j(ind_j), m_material(material), m_mercator(merc_state) {
        
    }

    FaceMesh::~FaceMesh() {
    
    }

    void FaceMesh::_ready() {
        generate_mesh();
    }

    void FaceMesh::generate_mesh() {
        m_array_mesh.instantiate();

        PackedVector3Array vertices;
        PackedInt32Array indices;
        PackedVector2Array uvs;
        PackedColorArray colors;
        PackedVector3Array normals;

        float step = 1.0f / m_mesh_per_face_res;
        float min_x = m_ind_i * step;
        float max_x = (m_ind_i + 1) * step;
        float min_y = m_ind_j * step;
        float max_y = (m_ind_j + 1) * step;

        //déterminer quel tile utiliser 
        Vector3 pos_start = Vector3(min_x - 0.5f, min_y - 0.5f, 0.5f);
        switch (m_direction) {
                    case FRONT:   // +Z
                        pos_start = Vector3(pos_start.x, pos_start.y, 0.5f);
                        break;
                    case BACK:    // -Z
                        pos_start = Vector3(-pos_start.x, pos_start.y, -0.5f);
                        break;
                    case LEFT:    // -X
                        pos_start = Vector3(-0.5f, pos_start.y, -pos_start.x);
                        break;
                    case RIGHT:   // +X
                        pos_start = Vector3(0.5f, pos_start.y, pos_start.x);
                        break;
                    case TOP:     // +Y
                        pos_start = Vector3(pos_start.x, 0.5f, -pos_start.y);
                        break;
                    case BOTTOM:  // -Y
                        pos_start = Vector3(pos_start.x, -0.5f, pos_start.y);
                        break;
                    default:
                        break;
            }
            Vector3 norm_pos_start = pos_start.normalized(); 
            float glob_u_start = 1.0f - (0.5f + (atan2(norm_pos_start.z, norm_pos_start.x) / (2.0f * Math_PI)));
            float glob_v_start = 0.5f - (asin(norm_pos_start.y) / Math_PI);
            //tile_x = std::min(static_cast<int>(glob_u_start * 8), 7); // 0 à 7 inclus
            //tile_y = std::min(static_cast<int>(glob_v_start * 4), 3); // 0 à 3 inclus
            tile_x = ((int)(glob_u_start * 8)) % 8;
            tile_y = 3 - (((int)(glob_v_start * 4)) % 4);
            std::string tile_path_std = "res://assets/tiles/" + std::to_string(tile_x) + "_" + std::to_string(tile_y) + ".jpg";
            //UtilityFunctions::print(String(tile_path_std.c_str()));
            
            Ref<Texture2D> tile_texture;
            auto it = tile_cache.find(tile_path_std);
            if (it != tile_cache.end()) {
                tile_texture = it->second;
            } else {
                tile_texture = ResourceLoader::get_singleton()->load(tile_path_std.c_str());
                if (tile_texture.is_valid()) {
                    //UtilityFunctions::print(tile_path_std.c_str());
                    tile_cache[tile_path_std] = tile_texture;
                }

            }

        // Générer une grille de (mesh_res+1)x(mesh_res+1) vertices sur le plan XY
        for (int y = 0; y <= m_mesh_res; ++y) {
            for (int x = 0; x <= m_mesh_res; ++x) {
                //float local_u = (float)x / m_mesh_res;
                //float local_v = (float)y / m_mesh_res;

                float fx = min_x + (max_x - min_x) * ((float)x / m_mesh_res);
                float fy = min_y + (max_y - min_y) * ((float)y / m_mesh_res);
                Vector3 pos = Vector3(fx - 0.5f, fy - 0.5f, 0.5f); // Face avant par défaut

                switch (m_direction) {
                    case FRONT:   // +Z
                        pos = Vector3(pos.x, pos.y, 0.5f);
                        break;
                    case BACK:    // -Z
                        pos = Vector3(-pos.x, pos.y, -0.5f);
                        break;
                    case LEFT:    // -X
                        pos = Vector3(-0.5f, pos.y, -pos.x);
                        break;
                    case RIGHT:   // +X
                        pos = Vector3(0.5f, pos.y, pos.x);
                        break;
                    case TOP:     // +Y
                        pos = Vector3(pos.x, 0.5f, -pos.y);
                        break;
                    case BOTTOM:  // -Y
                        pos = Vector3(pos.x, -0.5f, pos.y);
                        break;
                    default:
                        break;
            }
            Vector3 norm_pos = pos.normalized(); 
            Vector3 noise_pos = norm_pos *  noise(norm_pos);
            
            float alt_coef = (pos.length()-0.1f)*(pos.length()-0.1f);
            //colors.push_back(Color(alt_coef*0.5f, alt_coef*2.0f, alt_coef*0.75f, 1.0));
            colors.push_back(Color(0.5f, 1.0f, 0.5f, 1.0f));
            if (m_mercator) {
                vertices.push_back(to_mercator(noise_pos) * m_radius);
            } else {
                vertices.push_back(noise_pos * m_radius);
            }

            float glob_u = fmod(1.0f - (0.5f + (atan2(norm_pos.z, norm_pos.x) / (2.0f * Math_PI))), 1.0f);
            if (glob_u < 0.0f) glob_u += 1.0f;
            float glob_v = 0.5f - (asin(norm_pos.y) / Math_PI);
            glob_v = std::clamp(glob_v, 0.0f, 1.0f);

            float local_u = (glob_u * 8) - tile_x;
            float local_v = (glob_v * 4) - tile_y;
            
            uvs.push_back(Vector2(local_u, local_v));

        }
    }   

        for (int i = 0; i < vertices.size(); ++i) {
            normals.push_back(vertices[i].normalized());
        }

        // Générer les indices pour les triangles
        bool invert = (m_direction == RIGHT || m_direction == LEFT);
        for (int y = 0; y < m_mesh_res; ++y) {
            for (int x = 0; x < m_mesh_res; ++x) {
                int i = y * (m_mesh_res + 1) + x;
                if (!invert) {
                    indices.push_back(i);
                    indices.push_back(i + m_mesh_res + 1);
                    indices.push_back(i + 1);

                    indices.push_back(i + 1);
                    indices.push_back(i + m_mesh_res + 1);
                    indices.push_back(i + m_mesh_res + 2);
                } else {
                    // Inverser l'ordre pour que la normale pointe vers l'extérieur
                    indices.push_back(i);
                    indices.push_back(i + 1);
                    indices.push_back(i + m_mesh_res + 1);

                    indices.push_back(i + 1);
                    indices.push_back(i + m_mesh_res + 2);
                    indices.push_back(i + m_mesh_res + 1);
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

        if (tile_texture.is_valid()) {
            if (!m_material.is_valid()) {
                m_material.instantiate();
            }
            Ref<StandardMaterial3D> mat = m_material;
            mat->set_texture(StandardMaterial3D::TEXTURE_ALBEDO, tile_texture);
            set_surface_override_material(0, mat);
        }


        //set_surface_override_material(0, m_material);
    }

    void FaceMesh::generate_colors(Ref<StandardMaterial3D> material) {
        m_material = material;
        if (m_array_mesh.is_valid()) {
            m_array_mesh->surface_set_material(0, m_material);
        }
        
    }

    void FaceMesh::set_mercator(bool state){
        m_mercator=state;
    }

    void FaceMesh::switch_mercator_spherical_projection(bool state) {
        if (!m_array_mesh.is_valid()){
            UtilityFunctions::print("Mesh is not valid");
            return;
        }
        m_mercator = state;
        generate_mesh();
    }

    void FaceMesh::_bind_methods() {
    }
}