#pragma once

#include <algorithm>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector3.hpp>

using namespace godot;

// Conversion sphérique (x, y, z) -> Mercator (x', y', altitude)
inline Vector3 to_mercator(const Vector3 &v) {
    float r = v.length();
    float lat = std::asin(v.y / r); // latitude [-pi/2, pi/2]
    float lon = std::atan2(v.x, v.z); // longitude [-pi, pi]
    float x_merc = lon;
    float y_merc = std::log(std::tan(M_PI / 4.0f + lat / 2.0f));
    return Vector3(x_merc, y_merc, r); // r = altitude (distance au centre)
}

// Conversion Mercator (x', y', altitude) -> sphérique (x, y, z)
inline Vector3 from_mercator(const Vector3 &v) {
    float lon = v.x;
    float lat = 2.0f * std::atan(std::exp(v.y)) - M_PI / 2.0f;
    float r = v.z;
    float y = r * std::sin(lat);
    float x = r * std::sin(lon) * std::cos(lat);
    float z = r * std::cos(lon) * std::cos(lat);
    return Vector3(x, y, z);
}

inline Vector3 get_spherical_pos_from_flat_pos(Vector2 float_pos, float radius) {
    float longitude = float_pos.x * M_PI;
    float latitude = -(float_pos.y + 0.5) * M_PI;
    Vector3 spherical_pos = Vector3(
        -radius * sin(latitude) * cos(longitude),
        -radius * cos(latitude),
        radius * sin(latitude) * sin(longitude)
    );

    return spherical_pos;
}
