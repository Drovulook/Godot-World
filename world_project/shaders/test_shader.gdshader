shader_type spatial;
render_mode cull_back;

// Texture de la Terre (albedo)
uniform sampler2D albedo_texture;

// Paramètres pour le matériau
uniform float metallic = 0.0;
uniform float roughness = 1.0;

// Paramètres pour l'éclairage
uniform vec3 light_direction = vec3(0.0, 0.0, -1.5); // Direction de la lumière (ex. : soleil)
uniform vec3 light_color = vec3(1.0, 1.0, 1.0); // Couleur de la lumière (blanc)
uniform float light_intensity = 1.0; // Intensité de la lumière

void vertex() {
    // Les normales sont déjà calculées dans ton code C++
    // Pas besoin de modifications ici
}

void fragment() {
    // Récupérer la couleur de la texture
    vec3 albedo = texture(albedo_texture, UV).rgb;

    // Normaliser la direction de la lumière
    vec3 N = normalize(NORMAL);
    vec3 L = normalize(-light_direction);

    // Calculer l'éclairage diffus (Lambert)
    float NdotL = max(0.0, dot(N, L));
    vec3 diffuse = albedo * light_color * light_intensity * NdotL;

    // Appliquer le résultat
    ALBEDO = diffuse;
    METALLIC = metallic;
    ROUGHNESS = roughness;
}