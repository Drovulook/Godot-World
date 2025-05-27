#pragma once

#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/material.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>

#include <godot_cpp/classes/camera3d.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/classes/scene_tree.hpp>

#include <godot_cpp/classes/shader_material.hpp>
#include <godot_cpp/classes/shader.hpp>

#include "PlanetMesh.h"
#include "NCAltitudeReader.h"

#include <memory>
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

            void set_material(Ref<ShaderMaterial> material);
            Ref<ShaderMaterial> get_material() const;

            bool get_mercator() const;
            void set_mercator(bool state);
            
            void generate();
            void generate_colors();
        
        private:
            void create_texture_map();
                Camera3D* get_current_camera();
                bool is_tile_visible(int x, int y, Camera3D* camera);
                void generate_visible_meshes();
                void update_visible_meshes();
                void create_mesh_if_needed(int x, int y);
                void remove_mesh_if_exists(int x, int y);

        private:
            float m_radius = 5.0f;
            Ref<ArrayMesh> m_array_mesh;
            int m_mesh_per_img_res = 5;
            int m_mesh_res = 15;
            Ref<ShaderMaterial> m_material;

            bool m_mercator = false;

            std::vector<PlanetMesh*> m_meshes;

            std::unordered_map<std::string, Ref<Texture2D>> m_tile_cache;
            std::unordered_map<std::string, PlanetMesh*> m_active_meshes;

            std::shared_ptr<NCAltitudeReader> m_elevation_reader;

        protected:
            static void _bind_methods();
    };

}

