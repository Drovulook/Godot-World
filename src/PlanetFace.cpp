#include "PlanetFace.h"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    PlanetFace::PlanetFace(FaceDirection direction, float radius, int mesh_per_face_res, 
        int mesh_res, Ref<StandardMaterial3D> material, bool merc_state) 
        : m_direction(direction), m_radius(radius), m_mesh_per_face_res(mesh_per_face_res),
         m_mesh_res(mesh_res), m_material(material), m_mercator(merc_state) {

    }

    PlanetFace::~PlanetFace() {

    }

    void PlanetFace::_ready() {
        //generate();
    }

    void PlanetFace::generate() {
        for (auto mesh : m_meshes) {
            mesh->queue_free();
        }
        m_meshes.clear();

        for (int i = 0; i < m_mesh_per_face_res; ++i) {
            for (int j = 0; j < m_mesh_per_face_res; ++j) {
                
                FaceMesh* mesh = memnew(FaceMesh(m_direction, m_radius, m_mesh_res, 
                    m_mesh_per_face_res, i, j, m_material, m_mercator));
                add_child(mesh); // <-- AJOUT À LA SCÈNE
                m_meshes.push_back(mesh);
            }
        }
    }



    void PlanetFace::generate_colors(Ref<StandardMaterial3D> material) {
        m_material = material;
        for (auto mesh : m_meshes) {
            mesh->generate_colors(material);
        }
    }

    void PlanetFace::set_mercator(bool state){
        m_mercator = state;
    }

    void PlanetFace::_bind_methods() {

    }
}