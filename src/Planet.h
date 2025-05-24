#pragma once

#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/material.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>

#include "PlanetFace.h"

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

            void set_mesh_per_face_res(int res);
            int get_mesh_per_face_res() const;

            void set_mesh_res(int res);
            int get_mesh_res() const;

            void set_material(Ref<StandardMaterial3D> material);
            Ref<StandardMaterial3D> get_material() const;

            bool get_mercator() const;
            void set_mercator(bool state);
            
            void generate();
            void generate_colors();

        private:
            float radius = 5.0f;
            Ref<ArrayMesh> m_array_mesh;
            int m_meshPerFaceRes = 5;
            int m_meshRes = 10;
            std::vector<PlanetFace*> m_faces;
            Ref<StandardMaterial3D> m_material;

            bool mercator = false;

        protected:
            static void _bind_methods();
    };

}

