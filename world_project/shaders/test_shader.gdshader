shader_type spatial;
render_mode cull_back;

uniform sampler2D albedo_texture;
uniform float metallic : hint_range(0,1) = 0.0;
uniform float roughness : hint_range(0,1) = 1.0;
uniform float saturation : hint_range(0.0, 3.0) = 1.5;
uniform float brightness : hint_range(0.0, 2.0) = 1.2;
uniform float contrast : hint_range(0.0, 3.0) = 1.3;

void fragment() {
    vec3 color = texture(albedo_texture, UV).rgb;

    // Augmenter la luminosité
    color *= brightness;

    // Augmenter le contraste
    color = (color - 0.5) * contrast + 0.5;

    // Augmenter la saturation
    vec3 gray = vec3(dot(color, vec3(0.299, 0.587, 0.114)));
    color = mix(gray, color, saturation);

    // Clamp pour éviter les débordements
    color = clamp(color, 0.0, 1.0);
    ALBEDO = color;
    METALLIC = metallic;
    ROUGHNESS = roughness;
}