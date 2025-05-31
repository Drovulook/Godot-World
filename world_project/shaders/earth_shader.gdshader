shader_type spatial;
render_mode cull_back, diffuse_toon, specular_toon;

//texture
uniform sampler2D albedo_texture;
uniform float metallic : hint_range(0,1) = 0.0;
uniform float roughness : hint_range(0,1) = 0.8;

uniform float elevation_scale = 0.05;

varying float vertex_elevation;
varying float vertex_elevation_for_fragments;

// effet cartoon
uniform float saturation : hint_range(0.0, 3.0) = 1.6;
uniform float brightness : hint_range(0.0, 2.0) = 1.2;
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

// vertex
uniform float NOCHANGE_planet_radius;
uniform bool NOCHANGE_use_mercator ;

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
    
    float combined_wave = (wave1 + wave2 * 0.7 + wave3) * wave_amplitude;
    
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
	vertex_elevation_for_fragments = VERTEX.z;
    vertex_elevation = VERTEX.z;
	//vertex_elevation = VERTEX.z;

    float longitude = VERTEX.x * PI;
    float latitude = -(VERTEX.y + 0.5) * PI;

	if(vertex_elevation < -0.02) { //ocean
		vertex_elevation = calculate_waves(longitude, latitude, TIME);
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

    // Style cartoon : couleurs plus vives
    color *= brightness;

    // Saturation améliorée
    vec3 gray = vec3(dot(color, vec3(0.299, 0.587, 0.114)));
    color = mix(gray, color, saturation);

    // Rim lighting pour un effet cartoon
    float rim = 1.0 - dot(NORMAL, VIEW);
    rim = pow(rim, rim_power);
    color += rim_color * rim * 0.3;
	
	if (vertex_elevation_for_fragments < 0.0 && color.b > (color.r + color.g)/0.8) {
		//ALBEDO = vec3 (0.2, 0.2, 0.0);
		ALBEDO = clamp(color, 0.0, 1.0);
	    METALLIC = metallic;
		ROUGHNESS = 0.0;
	} else {
	//neige plus brillante
	float dynamic_roughness = roughness;
    float luminance = dot(color, vec3(0.299, 0.587, 0.114));
    float color_snow_mask = smoothstep(color_snow_threshold - 0.1, color_snow_threshold + 0.1, luminance);
    float altitude_snow_mask = smoothstep(altitude_snow_threshold - 0.1, altitude_snow_threshold + 0.1, vertex_elevation);
	float snow_mask = min(color_snow_mask, altitude_snow_mask);
	color = mix(color, color * snow_brightness, snow_mask);
    dynamic_roughness = mix(roughness, snow_roughness, snow_mask);

    ALBEDO = clamp(color, 0.0, 1.0);
    METALLIC = metallic;
    ROUGHNESS = dynamic_roughness;
	}
	
}