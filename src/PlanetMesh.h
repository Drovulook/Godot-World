#pragma once

#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/material.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/classes/texture2d.hpp>

#include <godot_cpp/classes/shader_material.hpp>
#include <godot_cpp/classes/shader.hpp>

namespace godot {
    class PlanetMesh : public MeshInstance3D {
        GDCLASS(PlanetMesh, MeshInstance3D);

        public:
            PlanetMesh(float radius, int mesh_res, Ref<ShaderMaterial> material,
                     bool mercator, Ref<Texture2D> tile, 
                     Vector2 bottom_left_corner_pos, Vector2 top_right_corner_pos);
            
            PlanetMesh() = default;
            ~PlanetMesh();

            void _ready() override;

            void generate_mesh();
            void generate_colors(Ref<ShaderMaterial> material);

            void set_mercator(bool state);

            void switch_mercator_spherical_projection(bool state);

        private:
            float m_radius;
            int m_mesh_res;
            Ref<ShaderMaterial> m_material;
            bool m_mercator;
            Ref<Texture2D> m_tile;
            Vector2 m_bottom_left_corner_pos;
            Vector2 m_top_right_corner_pos;

            Ref<ArrayMesh> m_array_mesh;

        protected:
            static void _bind_methods();
    };
}