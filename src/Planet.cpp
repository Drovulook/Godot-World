#include "Planet.h"
#include "FaceDirection.h"
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/engine.hpp>

namespace godot {
    Planet::Planet(){
        
    }

    Planet::~Planet(){

    }

    void Planet::_ready() {
        generate();
    }

    void Planet::_process(double delta){

    }

    void Planet::set_radius(float new_radius){
        radius = new_radius;
        generate();
    }

    float Planet::get_radius() const{
        return radius;
    }

    void Planet::set_mesh_per_face_res(int res){
        m_meshPerFaceRes = res;
        generate();
    }

    int Planet::get_mesh_per_face_res() const{
        return m_meshPerFaceRes;
    }

    void Planet::set_mesh_res(int res){
        m_meshRes = res;
        generate();
    }

    int Planet::get_mesh_res() const{
        return m_meshRes;
    }

    Ref<StandardMaterial3D> Planet::get_material() const{
        return m_material;
    }

    bool Planet::get_mercator() const{
        return mercator;
    }

    void Planet::set_mercator(bool state){
        mercator = state;
        generate();
    }

    void Planet::set_material(Ref<StandardMaterial3D> material){
        m_material = material;
        generate_colors();
    }

    void Planet::generate(){
        for (auto face : m_faces) {
            face->queue_free();
        }   
        m_faces.clear();
        for(int i = 0; i < 6; i++){
            PlanetFace* face = memnew(PlanetFace(FaceDirection(i), radius, m_meshPerFaceRes, m_meshRes, m_material, mercator));
            add_child(face); // <-- AJOUT À LA SCÈNE
            face->generate();
            m_faces.push_back(face);
        }
    }

    void Planet::generate_colors(){
        for (auto face : m_faces) {
            face->generate_colors(m_material);
            
        }
    }

    void Planet::_bind_methods(){
        ClassDB::bind_method(D_METHOD("set_radius", "new_radius"), &Planet::set_radius);
        ClassDB::bind_method(D_METHOD("get_radius"), &Planet::get_radius);
        ClassDB::bind_method(D_METHOD("set_mesh_per_face_res", "res"), &Planet::set_mesh_per_face_res);
        ClassDB::bind_method(D_METHOD("get_mesh_per_face_res"), &Planet::get_mesh_per_face_res);
        ClassDB::bind_method(D_METHOD("set_mesh_res", "res"), &Planet::set_mesh_res);
        ClassDB::bind_method(D_METHOD("get_mesh_res"), &Planet::get_mesh_res);
        ClassDB::bind_method(D_METHOD("set_material", "material"), &Planet::set_material);
        ClassDB::bind_method(D_METHOD("get_material"), &Planet::get_material);
        ClassDB::bind_method(D_METHOD("get_mercator"), &Planet::get_mercator);
        ClassDB::bind_method(D_METHOD("set_mercator", "state"), &Planet::set_mercator);

        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "radius"), "set_radius", "get_radius");
        ADD_PROPERTY(PropertyInfo(Variant::INT, "mesh_per_face_res", PROPERTY_HINT_RANGE, "1,50,1"), "set_mesh_per_face_res", "get_mesh_per_face_res");
        ADD_PROPERTY(PropertyInfo(Variant::INT, "mesh_res", PROPERTY_HINT_RANGE, "1,50,1"), "set_mesh_res", "get_mesh_res");
        ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "material", PROPERTY_HINT_RESOURCE_TYPE, "StandardMaterial3D"), "set_material", "get_material");
        ADD_PROPERTY(PropertyInfo(Variant::BOOL, "mercator"), "set_mercator", "get_mercator");
    }
}