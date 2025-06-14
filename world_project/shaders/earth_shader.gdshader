shader_type spatial;
render_mode cull_back, diffuse_toon, specular_toon;

#include "res://shaders/include/waves.gdshaderinc"
#include "res://shaders/include/alt_at_edges.gdshaderinc"

//texture
uniform sampler2D albedo_texture;
uniform sampler2D elevation_texture;
uniform sampler2D province_idx_texture;
uniform sampler2D city_idx_texture;

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

//provinces
uniform vec3 NOCHANGE_province_color_to_highlight = vec3(0.0, 0.0, 0.0);

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
		vertex_elevation = compute_avg_alt_at_edges(global_UV,
		calculate_waves(longitude, latitude, TIME, NOCHANGE_planet_radius, wave_speed, wave_amplitude, wave_frequency),
		elevation_texture, NOCHANGE_min_alt, NOCHANGE_max_alt);
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
	vec3 alt_color = texture(elevation_texture, global_UV).rgb;
	vec3 province_color = texture(province_idx_texture, transform_uv_local_to_global(UV, true)).rgb;
	vec3 city_color = texture(city_idx_texture, transform_uv_local_to_global(UV, true)).rgb;

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
		    float green_sat = (color.g - (color.r + color.b) * 0.2) * 0.2;
		    color.g = min(color.g + green_sat, 1.0);
	    }
	}
	float pv_color_threshold = 0.00003;
	if(NOCHANGE_province_color_to_highlight != vec3(0.0)&& ((province_color.r - NOCHANGE_province_color_to_highlight.r) < pv_color_threshold && (province_color.r - NOCHANGE_province_color_to_highlight.r) > -pv_color_threshold)
	 															&& ((province_color.g - NOCHANGE_province_color_to_highlight.g) < pv_color_threshold && (province_color.g - NOCHANGE_province_color_to_highlight.g) > -pv_color_threshold)
																&& ((province_color.b - NOCHANGE_province_color_to_highlight.b) < pv_color_threshold && (province_color.b - NOCHANGE_province_color_to_highlight.b) > -pv_color_threshold)
	)
	{
		color = color * 1.5;
	}

	if(city_color != vec3(0.0, 0.0, 0.0) && !(is_ocean_value > 0.5)) {
		color = color * 1.5 + vec3(0.1, 0.1, 0.1);
	}

    ALBEDO = clamp(color, 0.0, 1.0);
    METALLIC = metallic;
    ROUGHNESS = processed_roughness;

}