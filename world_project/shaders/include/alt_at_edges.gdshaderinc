float compute_avg_alt_at_edges(vec2 global_uv, float current_alt, sampler2D elevation_texture, float min_alt, float max_alt) {
        // Définir la largeur de la zone de bordure en coordonnées UV
    float border_width = 2.0 / 1320.0;

    // Calculer les dimensions d'une tuile en coordonnées UV
    float tile_width_uv = 1.0 / 8.0;  // 8 tuiles en X
    float tile_height_uv = 1.0 / 4.0; // 4 tuiles en Y

    // Position dans la tuile courante (0 à 1)
    vec2 tile_local_uv = mod(global_uv * vec2(8.0, 4.0), 1.0);

    // Vérifier si on est près d'un bord de tuile
    bool near_left = tile_local_uv.x < border_width;
    bool near_right = tile_local_uv.x > (1.0 - border_width);
    bool near_bottom = tile_local_uv.y < border_width;
    bool near_top = tile_local_uv.y > (1.0 - border_width);

    // Si on n'est pas près d'un bord, retourner l'altitude normale
    if (!near_left && !near_right && !near_bottom && !near_top) {
        return current_alt;
    }

	float texel_size = 1.0/1320.0;

	if(near_left) {
	    float left_alt = (texture(elevation_texture, clamp(global_uv + vec2(-texel_size, 0.0), 0.0, 1.0)).r
        * (max_alt - min_alt) + min_alt) / 9000.0;
		//return (current_alt + left_alt)/2.0;
		return min(current_alt, left_alt);
	}

	if(near_right) {
	    float right_alt = (texture(elevation_texture, clamp(global_uv + vec2(texel_size, 0.0), 0.0, 1.0)).r
        * (max_alt - min_alt) + min_alt) / 9000.0;
		//return (current_alt + right_alt)/2.0;
		return min(current_alt, right_alt);
	}

	if(near_top) {
		float top_alt = (texture(elevation_texture, clamp(global_uv + vec2(0.0, texel_size), 0.0, 1.0)).r
        * (max_alt - min_alt) + min_alt) / 9000.0;
		//return (current_alt + top_alt)/2.0;
		return min(current_alt, top_alt);
	}

	if(near_bottom) {
	    float bottom_alt = (texture(elevation_texture, clamp(global_uv + vec2(0.0, -texel_size), 0.0, 1.0)).r
        * (max_alt - min_alt) + min_alt) / 9000.0;
		//return (current_alt + bottom_alt)/2.0;
		return min(current_alt, bottom_alt);
	}

}