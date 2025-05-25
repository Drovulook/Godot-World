#pragma once

#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/material.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>

#include "PlanetMesh.h"

#include <vector>
#include <map>

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

            void set_material(Ref<StandardMaterial3D> material);
            Ref<StandardMaterial3D> get_material() const;

            bool get_mercator() const;
            void set_mercator(bool state);
            
            void generate();
            void generate_colors();
        
        private:
            void create_texture_map();

        private:
            float m_radius = 5.0f;
            Ref<ArrayMesh> m_array_mesh;
            int m_mesh_per_img_res = 5;
            int m_mesh_res = 15;
            Ref<StandardMaterial3D> m_material;

            bool m_mercator = false;

            std::vector<PlanetMesh*> m_meshes;

            std::map<std::string, Ref<Texture2D>> m_tile_cache;

        protected:
            static void _bind_methods();
    };

}

