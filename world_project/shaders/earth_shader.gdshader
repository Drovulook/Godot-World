shader_type spatial;
render_mode cull_back, diffuse_toon, specular_toon;

//texture
uniform sampler2D albedo_texture;
uniform sampler2D province_idx_texture;
uniform sampler2D elevation_texture;

uniform float metallic : hint_range(0,1) = 0.0;
uniform float roughness : hint_range(0,1) = 0.8;

uniform float elevation_scale = 0.05;

varying float vertex_elevation_for_fragments;
varying float is_ocean_value;
varying vec2 global_UV;

// effet cartoon
uniform float saturation : hint_range(0.0, 3.0) = 1.6;
uniform float brightness : hint_range(0.0, 10.0) = 3;
uniform float rim_power : hint_range(0.0, 10.0) = 2.0;
uniform vec3 rim_color : source_color = vec3(1.0, 1.0, 1.0);

// neige
uniform float color_snow_threshold : hint_range(0.5, 1.0) = 0.85; // Seuil de détection du blanc
uniform float altitude_snow_threshold : hint_range(0.1, 1.0) = 0.3;
uniform float snow_brightness : hint_range(1.0, 50.0) = 1.5;
uniform float snow_roughness : hint_range(0.0, 1.0) = 0.3;

//océan
uniform float wave_speed : hint_range(0.1, 3.0) = 1.0;
uniform float wave_amplitude : hint_range(0.0, 0.1) = 0.003;
uniform float wave_frequency : hint_range(1.0, 1000.0) = 100.0;
uniform vec3 ocean_deep_color : source_color = vec3(0.0, 0.0, 0.5);
uniform float ocean_specular : hint_range(0.0, 1.0) = 0.5;
uniform float ocean_fresnel : hint_range(0.0, 5.0) = 2.0;

// vertex
uniform float NOCHANGE_planet_radius;
uniform bool NOCHANGE_use_mercator;
uniform float NOCHANGE_min_alt = -0.1;
uniform float NOCHANGE_max_alt = 0.1;
uniform float NOCHANGE_tile_x = 0;
uniform float NOCHANGE_tile_y = 0;
uniform float NOCHANGE_sub_x = 0.0;
uniform float NOCHANGE_sub_y = 0.0;
uniform float NOCHANGE_mesh_res = 1.0;
uniform float NOCHANGE_img_res_in_mesh = 1.0;

vec2 transform_uv_local_to_global(vec2 uv, bool reverse_y) {
	float res_x = (NOCHANGE_tile_x + uv.x)/8.0;
	float res_y = (NOCHANGE_tile_y + (1.0 - uv.y))/4.0;
	if (reverse_y) {
		return vec2(res_x, 1.0 - res_y);
	}
	else {
		return vec2(res_x, res_y);
	}
}

float compute_avg_alt_at_edges(vec2 global_uv, float current_alt) {
        // Définir la largeur de la zone de bordure en coordonnées UV
    float border_width = 2.0 / 4320.0;

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

	float texel_size = 1.0/4320.0;

	if(near_left) {
	    float left_alt = (texture(elevation_texture, clamp(global_uv + vec2(-texel_size, 0.0), 0.0, 1.0)).r
        * (NOCHANGE_max_alt - NOCHANGE_min_alt) + NOCHANGE_min_alt) / 9000.0;
		//return (current_alt + left_alt)/2.0;
		return min(current_alt, left_alt);
	}

	if(near_right) {
	    float right_alt = (texture(elevation_texture, clamp(global_uv + vec2(texel_size, 0.0), 0.0, 1.0)).r
        * (NOCHANGE_max_alt - NOCHANGE_min_alt) + NOCHANGE_min_alt) / 9000.0;
		//return (current_alt + right_alt)/2.0;
		return min(current_alt, right_alt);
	}

	if(near_top) {
		float top_alt = (texture(elevation_texture, clamp(global_uv + vec2(0.0, texel_size), 0.0, 1.0)).r
        * (NOCHANGE_max_alt - NOCHANGE_min_alt) + NOCHANGE_min_alt) / 9000.0;
		//return (current_alt + top_alt)/2.0;
		return min(current_alt, top_alt);
	}

	if(near_bottom) {
	    float bottom_alt = (texture(elevation_texture, clamp(global_uv + vec2(0.0, -texel_size), 0.0, 1.0)).r
        * (NOCHANGE_max_alt - NOCHANGE_min_alt) + NOCHANGE_min_alt) / 9000.0;
		//return (current_alt + bottom_alt)/2.0;
		return min(current_alt, bottom_alt);
	}

}

float calculate_waves(float longitude, float latitude, float time) {
    float wave_offset = 0.0;

    // Vague principale basée sur longitude et latitude
    float wave1 = sin(longitude * wave_frequency + time * wave_speed) *
                  cos(latitude * wave_frequency * 0.7 + time * wave_speed * 1.2);

    // Vague secondaire perpendiculaire
    float wave2 = sin(latitude * wave_frequency * 1.3 + time * wave_speed * 0.8) *
                  cos(longitude * wave_frequency * 0.9 + time * wave_speed * 1.5);

    // Petites ondulations
    float wave3 = sin(longitude * wave_frequency * 2.5 + time * wave_speed * 2.0) *
                  sin(latitude * wave_frequency * 2.8 + time * wave_speed * 1.8) * 0.3;

	float wave4 = sin(longitude * wave_frequency * 8.0 + time * wave_speed * 3.0) *
                  cos(latitude * wave_frequency * 6.0 + time * wave_speed * 2.5) * 0.1;

    float combined_wave = (wave1 + wave2 * 0.7 + wave3 + wave4) * wave_amplitude;

    // Calculer la normale à la surface de la sphère à partir de longitude/latitude
    vec3 surface_normal = vec3(
        -sin(latitude) * cos(longitude),
        -cos(latitude),
        sin(latitude) * sin(longitude)
    );

    wave_offset = combined_wave * NOCHANGE_planet_radius;

    return wave_offset;
}

void vertex() {
	vec3 color_for_vertex = texture(albedo_texture, UV).rgb;

	global_UV = transform_uv_local_to_global(UV, false);
	float vertex_elevation = (texture(elevation_texture, global_UV).r
		* (NOCHANGE_max_alt - NOCHANGE_min_alt) + NOCHANGE_min_alt) / 9000.0;
	//float vertex_elevation = texture(elevation_texture, transform_uv_local_to_global(UV)).r;
	vertex_elevation_for_fragments = vertex_elevation;
    //VERTEX.z = vertex_elevation;


    float longitude = VERTEX.x * PI;
    float latitude = -(VERTEX.y + 0.5) * PI;

	is_ocean_value = (vertex_elevation < -0.02
		&& color_for_vertex.b > color_for_vertex.r*1.3
		&& color_for_vertex.b > color_for_vertex.g*1.3) ? 1.0 : 0.0;

	if(is_ocean_value > 0.5) { //ocean
		vertex_elevation = compute_avg_alt_at_edges(global_UV, calculate_waves(longitude, latitude, TIME));
	}

    if (NOCHANGE_use_mercator) {
        VERTEX = vec3 (
            VERTEX.x * NOCHANGE_planet_radius,
            VERTEX.y * NOCHANGE_planet_radius,
            vertex_elevation * elevation_scale * NOCHANGE_planet_radius
        );
    } else {
        float r = (1.0 + vertex_elevation * elevation_scale) * NOCHANGE_planet_radius;

        VERTEX = vec3 (
            - r * sin(latitude) * cos(longitude),
            -r * cos(latitude),
            r * sin(latitude) * sin(longitude)
        );
    }

    NORMAL = normalize(VERTEX);
}

void fragment() {
    vec3 color = texture(albedo_texture, UV).rgb;
	vec3 province_color = texture(province_idx_texture, transform_uv_local_to_global(UV, true)).rgb;
	vec3 alt_color = texture(elevation_texture, global_UV).rgb;

	float processed_roughness = roughness;

    // Style cartoon : couleurs plus vives
    color *= brightness;

    // Saturation améliorée
    vec3 gray = vec3(dot(color, vec3(0.299, 0.587, 0.114)));
    color = mix(gray, color, saturation);

    // Rim lighting pour un effet cartoon
    float rim = 1.0 - dot(NORMAL, VIEW);
    rim = pow(rim, rim_power);
	if (!NOCHANGE_use_mercator){
	    color += rim_color * rim * 0.3;
	}

	//océan
	if (is_ocean_value > 0.5) {
        float depth_factor = clamp((vertex_elevation_for_fragments + 0.50) / 0.5, 0.0, 1);
        vec3 ocean_color = mix(ocean_deep_color, color, depth_factor);
        if(!NOCHANGE_use_mercator) {
			float wave_highlight = pow(dot(NORMAL, vec3(0.0, 1.0, 0.0)), 8.0) * 0.3;
	        float fresnel = pow(rim, ocean_fresnel);
	        ocean_color = mix(ocean_color, vec3(1.0), fresnel * 0.2);
	        ocean_color += wave_highlight;
		}
		color = ocean_color;
		processed_roughness = 0.0;
	} else {
		//neige plus brillante
	    float luminance = dot(color, vec3(0.299, 0.587, 0.114));
	    float color_snow_mask = smoothstep(color_snow_threshold - 0.1, color_snow_threshold + 0.1, luminance);
	    float altitude_snow_mask = smoothstep(altitude_snow_threshold - 0.1, altitude_snow_threshold + 0.1, vertex_elevation_for_fragments);
		float snow_mask = min(color_snow_mask, altitude_snow_mask);
		color = mix(color, color * snow_brightness, snow_mask);
	    processed_roughness = mix(roughness, snow_roughness, snow_mask);

	    if (color.g > color.r * 1.2 && color.g > color.b * 1.2) {
		    // Saturation supplémentaire pour les verts
		    float green_sat = (color.g - (color.r + color.b) * 0.1) * 0.1;
		    color.g = min(color.g + green_sat, 1.0);
	    }
	}

    ALBEDO = clamp(vec3(province_color.b/3.0, province_color.b/3.0, province_color.b/3.0), 0.0, 1.0);
    METALLIC = metallic;
    ROUGHNESS = processed_roughness;

}