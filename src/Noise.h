#pragma once

#include <algorithm>
#include <godot_cpp/variant/vector3.hpp>

inline float noise(godot::Vector3 vec) {

    //return 0.995 + 0.055f * std::sin(30*vec.x) * std::cos(30*vec.y) * std::sin(30*vec.z);
    return 1;

}