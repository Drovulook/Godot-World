#pragma once

#include "FaceMesh.h"
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/material.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>

namespace godot {
    class PlanetFace : public Node3D {
        GDCLASS(PlanetFace, Node3D);
            public:
                PlanetFace(FaceDirection direction, float radius, 
                    int mesh_per_face_res, int mesh_res, Ref<StandardMaterial3D> material, bool merc_state);
                PlanetFace() = default;
                ~PlanetFace();
                void _ready() override;

                void generate();
                void generate_colors(Ref<StandardMaterial3D> material);

                void set_mercator(bool state);
            
            private:
                FaceDirection m_direction;
                float m_radius;
                int m_mesh_per_face_res;
                int m_mesh_res;
                std::vector<FaceMesh*> m_meshes;
                Ref<StandardMaterial3D> m_material;

                bool m_mercator;

            protected:
                static void _bind_methods();



    };
}