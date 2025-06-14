#include "CitiesManager.h"

namespace godot {
    CitiesManager::CitiesManager(Ref<Texture2D> city_texture, DebugUI *debug_ui)
   : m_cities_image(city_texture->get_image()), m_debug_ui(debug_ui) {}

    CitiesManager::~CitiesManager() {}

    void CitiesManager::_ready() {}

    void CitiesManager::load_city_mapping() {}

    void CitiesManager::decompress_image() {}

    void CitiesManager::_input(const Ref<InputEvent> &event) {}

    void CitiesManager::_bind_methods() {}
}