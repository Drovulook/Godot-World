#include "NetCDFReader.h"
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cmath>
#include "NCReader.h"

using namespace godot;

NCReader::NCReader(std::string file_path) : m_file_path(file_path) m_width(0), m_height(0), m_data_loaded(false) {

}

NCReader::~NCReader(){

}

bool NCReader::is_data_loaded() const{
    return m_data_loaded;
}

Vector2i NCReader::get_data_dimensions() const{
    return Vector2i(m_width, m_height);
}

Vector2 NCReader::get_lat_lon_range() const{
    if (!m_data_loaded || m_latitudes.empty() || m_longitudes.empty()) {
            return Vector2(0, 0);
        }

    float min_lat = *std::min_element(m_latitudes.begin(), m_latitudes.end());
    float max_lat = *std::max_element(m_latitudes.begin(), m_latitudes.end());
    float min_lon = *std::min_element(m_longitudes.begin(), m_longitudes.end());
    float max_lon = *std::max_element(m_longitudes.begin(), m_longitudes.end());
    
    UtilityFunctions::print("Lat range: ", min_lat, " to ", max_lat);
    UtilityFunctions::print("Lon range: ", min_lon, " to ", max_lon);
    
    return Vector2(max_lat - min_lat, max_lon - min_lon);
}

bool NCReader::load_netcdf_file(const String &file_path){
    UtilityFunctions::print("Attempting to load NetCDF file: ", file_path);
    
    // Vérifier si le fichier existe
    Ref<FileAccess> file = FileAccess::open(file_path, FileAccess::READ);
    if (!file.is_valid()) {
        UtilityFunctions::print("ERROR: Cannot open file: ", file_path);
        return false;
    }
    file->close();
    
    // Parser le fichier NetCDF (version simplifiée)
    bool success = parse_netcdf_simple(m_file_path);
    
    if (success) {
        UtilityFunctions::print("NetCDF file loaded successfully!");
        print_data_info();
        print_sample_data(5);
    } else {
        UtilityFunctions::print("ERROR: Failed to parse NetCDF file");
    }
    
    return success;
}
