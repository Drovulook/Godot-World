#pragma once

#include <string>
#include <vector>
#include <netcdf>

#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector2i.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/image_texture.hpp>

using namespace godot;

class NCAltitudeReader {
public:
    NCAltitudeReader(std::string file_path);
    ~NCAltitudeReader();

    bool load_file();
    void create_elevation_texture();

    bool is_data_loaded() const;
    Vector2i get_data_dimensions() const;
    Vector2 get_lat_lon_range() const;

    Ref<ImageTexture> get_elevation_texture();
    float get_min_elevation();
    float get_max_elevation();

private:
    int m_subsample = 7;

    std::string m_file_path;
    std::vector<float> m_latitudes;
    std::vector<float> m_longitudes;
    std::vector<float> m_elevation_data_flat;
    Ref<ImageTexture> m_elevation_texture;  
    int m_width;
    int m_height;
    bool m_data_loaded;
    float m_min_elevation = 0.0f;
    float m_max_elevation = 0.0f;

};