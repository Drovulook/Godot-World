#pragma once

#include <string>
#include <vector>

using namespace godot;

class NCReader {
public:
    NCReader(std::string file_path);
    ~NCReader();

    bool is_data_loaded() const;

    Vector2i get_data_dimensions() const;
    Vector2 get_lat_lon_range() const;

    bool load_netcdf_file(const String& file_path);
    void print_data_info() const;
    void print_sample_data(int sample_size = 10) const;
    float get_elevation_at(float latitude, float longitude) const;

private:
    std::string m_file_path;
    std::vector<std::vector<float>> m_elevation_data;
    std::vector<float> m_latitudes;
    std::vector<float> m_longitudes;
    int m_width;
    int m_height;
    bool m_data_loaded;

    bool parse_netcdf_simple(const std::string& file_path);
        int find_nearest_index(const std::vector<float>& array, float value) const;

    
};