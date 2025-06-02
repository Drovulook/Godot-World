#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cmath>
#include "NCAltitudeReader.h"

using namespace godot;

NCAltitudeReader::NCAltitudeReader(std::string file_path) 
    : m_file_path(file_path), m_width(0), m_height(0), m_data_loaded(false) {

}

NCAltitudeReader::~NCAltitudeReader(){

}

bool NCAltitudeReader::load_file(){
    
    try {
        netCDF::NcFile dataFile(m_file_path, netCDF::NcFile::read);
        
        // dimensions
        netCDF::NcDim latDim = dataFile.getDim("lat");
        netCDF::NcDim lonDim = dataFile.getDim("lon");
        int total_height = latDim.getSize();
        int total_width = lonDim.getSize();
        UtilityFunctions::print("Total dimensions: ", total_width, " x ", total_height);
        int new_height = (total_height + m_subsample - 1) / m_subsample;
        int new_width = (total_width + m_subsample - 1) / m_subsample;
        m_height = new_height;
        m_width = new_width;

        // charger latitude et longitude
        netCDF::NcVar latVar = dataFile.getVar("lat");
        netCDF::NcVar lonVar = dataFile.getVar("lon");

        m_latitudes.resize(new_height);
        m_longitudes.resize(new_width);

        for (int i = 0; i < new_height; ++i) {
            std::vector<size_t> start = {static_cast<size_t>(i * m_subsample)};
            std::vector<size_t> count = {1};
            latVar.getVar(start, count, &m_latitudes[i]);
        }
        
        for (int i = 0; i < new_width; ++i) {
            std::vector<size_t> start = {static_cast<size_t>(i * m_subsample)};
            std::vector<size_t> count = {1};
            lonVar.getVar(start, count, &m_longitudes[i]);
        }
        // Charger les données d'élévation
        netCDF::NcVar elevVar = dataFile.getVar("elevation");
        m_elevation_data_flat.resize(new_height * new_width);

        m_min_elevation = std::numeric_limits<float>::max();
        m_max_elevation = std::numeric_limits<float>::lowest();
        
        for (int yy = 0; yy < new_height; ++yy) {
            int y_orig = yy * m_subsample;
            if (y_orig >= total_height) break;
            
            // Charger une ligne complète
            std::vector<float> line_data(total_width);
            std::vector<size_t> start = {static_cast<size_t>(y_orig), 0};
            std::vector<size_t> count = {1, static_cast<size_t>(total_width)};
            elevVar.getVar(start, count, line_data.data());
            
            // Sous-échantillonner cette ligne
            for (int xx = 0; xx < new_width; ++xx) {
                int x_orig = xx * m_subsample;
                if (x_orig >= total_width) break;
                
                float elevation = line_data[x_orig];
                m_elevation_data_flat[yy * new_width + xx] = elevation;
                
                m_min_elevation = std::min(m_min_elevation, elevation);
                m_max_elevation = std::max(m_max_elevation, elevation);
            }
        }

        m_data_loaded = true;

        UtilityFunctions::print("Dimensions: ", m_width, " x ", m_height);
        UtilityFunctions::print("Elevation range: ", m_min_elevation, " to ", m_max_elevation);
        
        return true;

    } catch (netCDF::exceptions::NcException& e) {
        UtilityFunctions::print("NetCDF error: ", e.what());
        return false;
    }
    
}

void NCAltitudeReader::create_elevation_texture(){
    if (!m_data_loaded) {
        UtilityFunctions::print("No elevation data loaded");
        return;
    }

    // Créer une image avec les données d'altitude
    Ref<Image> image = Image::create(m_width, m_height, false, Image::FORMAT_RF);
    
    PackedByteArray data;
    data.resize(m_width * m_height * sizeof(float));
    
    for (int y = 0; y < m_height; y++) {
        for (int x = 0; x < m_width; x++) {
            float elevation = m_elevation_data_flat[y * m_width + x];
            // Normaliser l'altitude entre 0 et 1
            float normalized = (elevation - m_min_elevation) / (m_max_elevation - m_min_elevation);
            
            // Convertir en bytes
            float* pixel_ptr = reinterpret_cast<float*>(data.ptrw() + (y * m_width + x) * sizeof(float));
            *pixel_ptr = normalized;
        }
    }

    image->set_data(m_width, m_height, false, Image::FORMAT_RF, data);
    m_elevation_texture = ImageTexture::create_from_image(image);
}

bool NCAltitudeReader::is_data_loaded() const{
    return m_data_loaded;
}

Vector2i NCAltitudeReader::get_data_dimensions() const{
    return Vector2i(m_width, m_height);
}

Vector2 NCAltitudeReader::get_lat_lon_range() const{
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

Ref<ImageTexture> NCAltitudeReader::get_elevation_texture(){
    return m_elevation_texture;
}

float NCAltitudeReader::get_min_elevation(){
    return m_min_elevation;
}

float NCAltitudeReader::get_max_elevation(){
    return m_max_elevation;
}
