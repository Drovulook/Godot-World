float calculate_waves(float longitude, float latitude, float time, float planet_radius, float wave_speed, float wave_amplitude, float wave_frequency) {
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

    wave_offset = combined_wave * planet_radius;

    return wave_offset;
}