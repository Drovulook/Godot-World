#pragma once

#include <string>
#include <vector>
#include <netcdf>

#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector2i.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/classes/node.hpp>

using namespace godot;

class NCAltitudeReader {
public:
    NCAltitudeReader(std::string file_path);
    ~NCAltitudeReader();

    bool load_file();

    bool is_data_loaded() const;

    Vector2i get_data_dimensions() const;
    Vector2 get_lat_lon_range() const;

    float get_elevation_at(float latitude, float longitude) const;

private:
    int m_subsample = 3;

    std::string m_file_path;
    std::vector<std::vector<float>> m_elevation_data;
    std::vector<float> m_latitudes;
    std::vector<float> m_longitudes;
    int m_width;
    int m_height;
    bool m_data_loaded;
    float m_min_elevation;
    float m_max_elevation;

    
};