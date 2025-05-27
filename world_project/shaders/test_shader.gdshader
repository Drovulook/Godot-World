shader_type spatial;
render_mode cull_back;

uniform sampler2D albedo_texture;
uniform float metallic : hint_range(0,1) = 0.0;
uniform float roughness : hint_range(0,1) = 1.0;

void fragment() {
    // Juste assigner la texture - Godot s'occupe de l'éclairage
    ALBEDO = texture(albedo_texture, UV).rgb;
    METALLIC = metallic;
    ROUGHNESS = roughness;
}