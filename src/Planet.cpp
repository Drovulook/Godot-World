#include "Planet.h"
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/camera3d.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/classes/project_settings.hpp>

namespace godot {
    Planet::Planet(){

    }

    Planet::~Planet(){

    }

    void Planet::_ready() {
        UtilityFunctions::print("Planet::_ready() called");

        UtilityFunctions::print("Planet::_ready() - Initializing NCReader");
        String godot_path = "res://assets/earth_data.nc";
        String abs_path = ProjectSettings::get_singleton()->globalize_path(godot_path);
        std::string file_path = abs_path.utf8().get_data();
        m_elevation_reader = std::make_shared<NCAltitudeReader>(file_path);
        if (!m_elevation_reader->load_file()) {
            UtilityFunctions::print("Failed to load elevation data!");
            m_elevation_reader = nullptr;
        } else {
            UtilityFunctions::print("Elevation data loaded successfully!");
        }

        set_process(true);
        create_texture_map();
        generate_visible_meshes();

        m_initialized = true;
    }

    void Planet::_process(double delta){
        static int frame_counter = 0;
        if (++frame_counter % 10 == 0) { // Vérifier tous les 10 frames
            update_visible_meshes();
        }
    }

    void Planet::set_radius(float new_radius){
        m_radius = new_radius;
        if(m_initialized) generate();
    }

    float Planet::get_radius() const{
        return m_radius;
    }

    void Planet::set_mesh_per_img_res(int res){
        m_mesh_per_img_res = res;
        if(m_initialized) generate();
    }

    int Planet::get_mesh_per_img_res() const{
        return m_mesh_per_img_res;
    }

    void Planet::set_mesh_res(int res){
        m_mesh_res = res;
        if(m_initialized) generate();
    }

    int Planet::get_mesh_res() const{
        return m_mesh_res;
    }

    bool Planet::get_mercator() const{
        return m_mercator;
    }

    void Planet::set_mercator(bool state){
        m_mercator = state;
        if(m_initialized) generate();
    }

    void Planet::set_material(Ref<ShaderMaterial> material){
        m_material = material;

        if (m_material.is_valid() && m_initialized) {
        generate_visible_meshes();
        }
    }

    Ref<ShaderMaterial> Planet::get_material() const{
        return m_material;
    }

    void Planet::generate() {
        //create_texture_map();

        for (auto& pair : m_active_meshes) {
            pair.second->queue_free();
        }
        m_active_meshes.clear();

       generate_visible_meshes();

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

    Camera3D *Planet::get_current_camera(){
        // Obtenir la caméra actuelle (éditeur ou jeu)
        if (Engine::get_singleton()->is_editor_hint()) {
            // En mode éditeur, chercher la caméra de l'éditeur
            return nullptr; // L'éditeur nécessite une approche différente
        } else {
            // En mode jeu
            Viewport* viewport = get_viewport();
            if (viewport) {
                Camera3D* camera = viewport->get_camera_3d();
                return camera;
            }
        }
        return nullptr;
    }

    bool Planet::is_tile_visible(int x, int y, Camera3D *camera){
        if (!camera) return true; // Si pas de caméra, afficher tout
        
        // Calculer le centre du tile
        Vector2 bottom_left = Vector2(2*(x/8.0f - 0.5f), y/4.0f - 0.5f);
        Vector2 top_right = Vector2(2*((x+1)/8.0f - 0.5f), (y+1)/4.0f - 0.5f);
        Vector2 tile_center = (bottom_left + top_right) * 0.5f;
        
        Vector3 world_center;
        if (m_mercator) {
            world_center = Vector3(tile_center.x * m_radius, tile_center.y * m_radius, 0);
        } else {
            float longitude = tile_center.x * 2.0f * M_PI;
            float latitude = (tile_center.y - 0.5f) * M_PI;
            world_center = Vector3(
                m_radius * cos(latitude) * cos(longitude),
                m_radius * sin(latitude),
                m_radius * cos(latitude) * sin(longitude)
            );
        }
        
        // Transformer en coordonnées globales
        world_center = get_global_transform().xform(world_center);
        
        Vector3 cam_pos = camera->get_global_transform().origin;
        float dist = world_center.distance_to(cam_pos);
        // Seuil de proximité (ajuste selon tes besoins)
        float proximity_threshold = m_radius * 2.2f;

        // Vérifier si dans le frustum de la caméra
        return camera->is_position_in_frustum(world_center) || dist < proximity_threshold;
    }

    void Planet::generate_visible_meshes(){
        Camera3D* camera = get_current_camera();
        
        for (int x = 0; x < 8; x++) {
            for (int y = 0; y < 4; y++) {
                if (is_tile_visible(x, y, camera)) {
                    create_mesh_if_needed(x, y);
                } else {
                    remove_mesh_if_exists(x, y);
                }
            }
        }
    }

    void Planet::update_visible_meshes(){
        generate_visible_meshes();
    }

    void Planet::create_mesh_if_needed(int x, int y){

        std::string tile_id = std::to_string(x) + "_" + std::to_string(y);
        
        // Vérifier si le mesh existe déjà
        auto mesh_it = m_active_meshes.find(tile_id);
        if (mesh_it != m_active_meshes.end()) {
            return; // Mesh déjà créé
        }

        if (!m_material.is_valid()) {
            //UtilityFunctions::print("Material not ready, skipping mesh creation for tile: ", tile_id.c_str());
            return;
        }
        
        // Créer le mesh
        Vector2 bottom_left_corner_pos = Vector2(2*(x/8.0f - 0.5f), y/4.0f - 0.5f);
        Vector2 top_right_corner_pos = Vector2(2*((x+1)/8.0f - 0.5f), (y+1)/4.0f - 0.5f);
        
        Ref<Texture2D> tile;
        auto texture_it = m_tile_cache.find(tile_id);
        if (texture_it != m_tile_cache.end()) {
            tile = texture_it->second;
        }
        
        PlanetMesh* mesh = memnew(PlanetMesh(m_radius, m_mesh_res, m_material,
             m_mercator, tile, bottom_left_corner_pos, top_right_corner_pos, m_elevation_reader));
        add_child(mesh);
        m_active_meshes[tile_id] = mesh;
    }

    void Planet::remove_mesh_if_exists(int x, int y){
        std::string tile_id = std::to_string(x) + "_" + std::to_string(y);
        
        auto mesh_it = m_active_meshes.find(tile_id);
        if (mesh_it != m_active_meshes.end()) {
            mesh_it->second->queue_free();
            m_active_meshes.erase(mesh_it);
        }
    }

    void Planet::_bind_methods(){
        ClassDB::bind_method(D_METHOD("set_radius", "new_radius"), &Planet::set_radius);
        ClassDB::bind_method(D_METHOD("get_radius"), &Planet::get_radius);
        ClassDB::bind_method(D_METHOD("set_mesh_per_img_res", "res"), &Planet::set_mesh_per_img_res);
        ClassDB::bind_method(D_METHOD("get_mesh_per_img_res"), &Planet::get_mesh_per_img_res);
        ClassDB::bind_method(D_METHOD("set_mesh_res", "res"), &Planet::set_mesh_res);
        ClassDB::bind_method(D_METHOD("get_mesh_res"), &Planet::get_mesh_res);
        ClassDB::bind_method(D_METHOD("set_material", "m_material"), &Planet::set_material);
        ClassDB::bind_method(D_METHOD("get_material"), &Planet::get_material);
        ClassDB::bind_method(D_METHOD("get_mercator"), &Planet::get_mercator);
        ClassDB::bind_method(D_METHOD("set_mercator", "state"), &Planet::set_mercator);

        ClassDB::bind_method(D_METHOD("generate"), &Planet::generate);

        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "radius"), "set_radius", "get_radius");
        ADD_PROPERTY(PropertyInfo(Variant::INT, "mesh_per_img_res", PROPERTY_HINT_RANGE, "1,50,1"), "set_mesh_per_img_res", "get_mesh_per_img_res");
        ADD_PROPERTY(PropertyInfo(Variant::INT, "mesh_res", PROPERTY_HINT_RANGE, "1,800,1"), "set_mesh_res", "get_mesh_res");
        ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "m_material", PROPERTY_HINT_RESOURCE_TYPE, "ShaderMaterial"), "set_material", "get_material");
        ADD_PROPERTY(PropertyInfo(Variant::BOOL, "m_mercator"), "set_mercator", "get_mercator");
    }
}