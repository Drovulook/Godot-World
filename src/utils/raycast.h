#pragma once

#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/classes/camera3d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include "../Planet.h"

using namespace godot;

inline Vector2 sphere_raycast(Vector2 screen_pos, Node* context_node) {
  if (!context_node) {
      UtilityFunctions::print("No context node provided");
      return Vector2(-1, -1);
  }

  Viewport *viewport = context_node->get_viewport();
  if (!viewport) {
    UtilityFunctions::print("No viewport found");
    return Vector2(-1, -1);
  }

  Camera3D *camera = viewport->get_camera_3d();
  if (!camera) {
    UtilityFunctions::print("No camera found");
    return Vector2(-1, -1);
  }

  // Obtenir la planète (parent de ce manager)
  Planet *planet = Object::cast_to<Planet>(context_node->get_parent());
  if (!planet) {
    UtilityFunctions::print("No planet found as parent");
    return Vector2(-1, -1);
  }

  Vector3 ray_origin = camera->project_ray_origin(screen_pos);
  Vector3 ray_direction = camera->project_ray_normal(screen_pos);

  // Position et rayon de la planète
  Vector3 planet_center = planet->get_global_transform().origin;
  float planet_radius =
      planet->get_radius(); // Utiliser la valeur par défaut du radius

  // Intersection rayon-sphère
  Vector3 to_center = ray_origin - planet_center;
  float a = ray_direction.dot(ray_direction);
  float b = 2.0f * to_center.dot(ray_direction);
  float c = to_center.dot(to_center) - planet_radius * planet_radius;

  float discriminant = b * b - 4 * a * c;

  if (discriminant < 0) {
    return Vector2(-1, -1); // Pas d'intersection
  }

  // Prendre l'intersection la plus proche
  float t = (-b - sqrt(discriminant)) / (2 * a);
  if (t < 0) {
    t = (-b + sqrt(discriminant)) / (2 * a);
  }

  if (t < 0) {
    return Vector2(-1, -1);
  }

  // Point d'intersection sur la sphère
  Vector3 hit_point = ray_origin + ray_direction * t;

  // Convertir en coordonnées locales de la planète
  Vector3 local_hit =
      planet->get_global_transform().affine_inverse().xform(hit_point);

  // Convertir en coordonnées UV (projection équirectangulaire)
  Vector3 normalized = local_hit.normalized();

  // Calculer longitude et latitude
  float longitude = atan2(normalized.z, -normalized.x);
  float latitude = asin(-normalized.y);

  // Convertir en coordonnées UV (0-1)
  Vector2 uv_coords;
  uv_coords.x = (longitude / M_PI + 1.0f) * 0.5f;
  uv_coords.y = (-latitude / M_PI + 0.5f);

  return uv_coords;
}