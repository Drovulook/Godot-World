shader_type spatial;
render_mode cull_back, diffuse_toon, specular_toon;

//texture
uniform sampler2D albedo_texture;
uniform float metallic : hint_range(0,1) = 0.0;
uniform float roughness : hint_range(0,1) = 0.8;

uniform float elevation_scale = 0.05;

varying float vertex_elevation;

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

// vertex
uniform float NOCHANGE_planet_radius;
uniform bool NOCHANGE_use_mercator ;

void vertex() {
    vertex_elevation = VERTEX.z;

    if (NOCHANGE_use_mercator) {
        VERTEX = vec3 (
            VERTEX.x * NOCHANGE_planet_radius,
            VERTEX.y * NOCHANGE_planet_radius,
            vertex_elevation * elevation_scale * NOCHANGE_planet_radius
        );
    } else {
        float longitude = VERTEX.x * PI;
        float latitude = -(VERTEX.y + 0.5) * PI;

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