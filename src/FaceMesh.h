#pragma once

#include <godot_cpp/classes/mesh_instance3d.hpp>
#include "FaceDirection.h"
#include <godot_cpp/classes/material.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <map>

namespace godot {
    class FaceMesh : public MeshInstance3D {
        GDCLASS(FaceMesh, MeshInstance3D);

        public:
            FaceMesh(FaceDirection direction, float radius, int mesh_res, int mesh_per_face_res, 
                int ind_i, int ind_j, Ref<StandardMaterial3D> material, bool merc_state);
            FaceMesh() = default;
            ~FaceMesh();

            void _ready() override;

            void generate_mesh();
            void generate_colors(Ref<StandardMaterial3D> material);

            void set_mercator(bool state);

            void switch_mercator_spherical_projection(bool state);

        private:
            FaceDirection m_direction;
            float m_radius;
            int m_mesh_res;
            int m_mesh_per_face_res;
            int m_ind_i;
            int m_ind_j;

            int tile_x;
            int tile_y;

            Ref<StandardMaterial3D> m_material;
            Ref<ArrayMesh> m_array_mesh;

            bool m_mercator = false;

            static std::map<std::string, Ref<Texture2D>> tile_cache;
        
        protected:
            static void _bind_methods();

    };
}