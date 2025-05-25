#include "Planet.h"
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/resource_loader.hpp>

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
        m_radius = new_radius;
        generate();
    }

    float Planet::get_radius() const{
        return m_radius;
    }

    void Planet::set_mesh_per_img_res(int res){
        m_mesh_per_img_res = res;
        generate();
    }

    int Planet::get_mesh_per_img_res() const{
        return m_mesh_per_img_res;
    }

    void Planet::set_mesh_res(int res){
        m_mesh_res = res;
        generate();
    }

    int Planet::get_mesh_res() const{
        return m_mesh_res;
    }

    Ref<StandardMaterial3D> Planet::get_material() const{
        return m_material;
    }

    bool Planet::get_mercator() const{
        return m_mercator;
    }

    void Planet::set_mercator(bool state){
        m_mercator = state;
        generate();
    }

    void Planet::set_material(Ref<StandardMaterial3D> material){
        m_material = material;
        generate_colors();
    }

    void Planet::generate() {
        create_texture_map();

        for (auto mesh : m_meshes) {
            mesh->queue_free();
        }
        m_meshes.clear();

        for (int x = 0; x < 8; x++) {
            for (int y = 0; y < 4; y++) {
                // utiliser m_mesh_per_img_res
                Vector2 bottom_left_corner_pos = Vector2(2*(x/8.0f - 0.5f), y/4.0f - 0.5f);
                Vector2 top_right_corner_pos = Vector2(2*((x+1)/8.0f - 0.5f), (y+1)/4.0f - 0.5f);
                
                std::string tile_key = std::to_string(x) + "_" + std::to_string(y);
                Ref<Texture2D> tile;
                
                // Vérifier si la texture existe dans le cache
                auto it = m_tile_cache.find(tile_key);
                if (it != m_tile_cache.end()) {
                    tile = it->second;
                    UtilityFunctions::print("Texture found for tile: ", tile_key.c_str());
                } else {
                    UtilityFunctions::print("No texture found for tile: ", tile_key.c_str());
                }

                PlanetMesh* mesh = memnew(PlanetMesh(m_radius, m_mesh_res, m_material,
                     m_mercator, tile, bottom_left_corner_pos, top_right_corner_pos));
                add_child(mesh); // <-- AJOUT À LA SCÈNE
                m_meshes.push_back(mesh);
            }
        }

    }

    void Planet::generate_colors(){

    }

    void Planet::create_texture_map(){
        // Create a texture map for the planet
        for (int x = 0; x < 8; ++x) {
            for (int y = 0; y < 4; ++y) {
                std::string tile_name = std::to_string(x) + "_" + std::to_string(y);
                std::string tile_path = "res://assets/tiles/" + tile_name + ".jpg";
                Ref<Texture2D> tile_texture = ResourceLoader::get_singleton()->load(tile_path.c_str());
                if (tile_texture.is_valid()) {
                    m_tile_cache[tile_name] = tile_texture;
                }
            }
        }
    }

    void Planet::_bind_methods(){
        ClassDB::bind_method(D_METHOD("set_radius", "new_radius"), &Planet::set_radius);
        ClassDB::bind_method(D_METHOD("get_radius"), &Planet::get_radius);
        ClassDB::bind_method(D_METHOD("set_mesh_per_img_res", "res"), &Planet::set_mesh_per_img_res);
        ClassDB::bind_method(D_METHOD("get_mesh_per_img_res"), &Planet::get_mesh_per_img_res);
        ClassDB::bind_method(D_METHOD("set_mesh_res", "res"), &Planet::set_mesh_res);
        ClassDB::bind_method(D_METHOD("get_mesh_res"), &Planet::get_mesh_res);
        ClassDB::bind_method(D_METHOD("set_material", "material"), &Planet::set_material);
        ClassDB::bind_method(D_METHOD("get_material"), &Planet::get_material);
        ClassDB::bind_method(D_METHOD("get_mercator"), &Planet::get_mercator);
        ClassDB::bind_method(D_METHOD("set_mercator", "state"), &Planet::set_mercator);

        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "radius"), "set_radius", "get_radius");
        ADD_PROPERTY(PropertyInfo(Variant::INT, "mesh_per_img_res", PROPERTY_HINT_RANGE, "1,50,1"), "set_mesh_per_img_res", "get_mesh_per_img_res");
        ADD_PROPERTY(PropertyInfo(Variant::INT, "mesh_res", PROPERTY_HINT_RANGE, "1,50,1"), "set_mesh_res", "get_mesh_res");
        ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "m_material", PROPERTY_HINT_RESOURCE_TYPE, "StandardMaterial3D"), "set_material", "get_material");
        ADD_PROPERTY(PropertyInfo(Variant::BOOL, "m_mercator"), "set_mercator", "get_mercator");
    }
}