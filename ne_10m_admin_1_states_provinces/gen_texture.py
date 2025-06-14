import geopandas as gpd
import numpy as np
from PIL import Image
import json
from shapely.geometry import Polygon, MultiPolygon

def create_province_texture_pixel_perfect(shapefile_path, output_path, width=8640, height=4320):
    """
    Créer une texture avec rastérisation manuelle pixel-parfaite
    """
    
    # Charger les données Natural Earth
    world = gpd.read_file(shapefile_path)
    print(f"Nombre de provinces/états : {len(world)}")
    
    # Créer un array numpy pour contrôle pixel-parfait
    img_array = np.zeros((height, width, 3), dtype=np.uint8)
    
    # Créer aussi un array d'ID pour éviter les conflits
    id_array = np.zeros((height, width), dtype=np.uint32)
    
    province_colors = {}
    province_ids = {}
    
    # Traitement direct des provinces
    global_province_id = 1
    
    for idx, (_, province) in enumerate(world.iterrows()):
        if idx % 500 == 0:
            print(f"Traitement : {idx}/{len(world)} provinces...")
        
        country_code = province.get('adm0_a3', 'UNK')
        country_name = province.get('admin', 'Unknown')
        province_name = province.get('name', f'Province_{idx}')
        province_type = province.get('type_en', 'Province')
        
        # Générer couleur unique
        r = (global_province_id >> 16) & 0xFF
        g = (global_province_id >> 8) & 0xFF  
        b = global_province_id & 0xFF
        
        if r == 0 and g == 0 and b == 0:
            r = 1
        
        color = (r, g, b)
        
        province_key = f"{country_code}_{province_name}"
        province_colors[province_key] = color
        province_ids[str(global_province_id)] = {
            'country_code': country_code,
            'country_name': country_name,
            'province_name': province_name,
            'province_type': province_type,
            'color': {'r': r, 'g': g, 'b': b}
        }
        
        # Rastériser la géométrie
        if province.geometry and not province.geometry.is_empty:
            try:
                rasterize_geometry_manual(
                    img_array, id_array, province.geometry, 
                    color, global_province_id, width, height
                )
            except Exception as e:
                print(f"Erreur pour {province_name}: {e}")
        
        global_province_id += 1
    
    print(f"\nTraitement terminé. Sauvegarde...")
    
    # Convertir en image PIL et sauvegarder
    img = Image.fromarray(img_array, 'RGB')
    img.save(output_path, format='PNG', compress_level=0)
    
    # Sauvegarder les mappings
    save_mappings_json(output_path, province_ids)
    
    # Vérifier les couleurs
    verify_pixel_perfect_colors(output_path, province_colors)
    
    return province_colors, province_ids

def rasterize_geometry_manual(img_array, id_array, geometry, color, province_id, width, height):
    """
    Rastérisation manuelle pixel-parfaite sans anti-aliasing
    """
    if geometry.geom_type == 'Polygon':
        polygons = [geometry]
    elif geometry.geom_type == 'MultiPolygon':
        polygons = list(geometry.geoms)
    else:
        return
    
    for polygon in polygons:
        if not polygon.is_valid:
            polygon = polygon.buffer(0)
            if not polygon.is_valid:
                continue
        
        # Convertir les coordonnées géographiques en pixels
        coords = []
        for x, y in polygon.exterior.coords:
            if -180 <= x <= 180 and -90 <= y <= 90:
                pixel_x = int(((x + 180.0) / 360.0) * width)
                pixel_y = int(((90.0 - y) / 180.0) * height)
                pixel_x = max(0, min(width - 1, pixel_x))
                pixel_y = max(0, min(height - 1, pixel_y))
                coords.append((pixel_x, pixel_y))
        
        if len(coords) < 3:
            continue
        
        # Rastérisation manuelle par scan-line
        fill_polygon_scanline(img_array, id_array, coords, color, province_id, width, height)
        
        # Traiter les trous
        for interior in polygon.interiors:
            hole_coords = []
            for x, y in interior.coords:
                if -180 <= x <= 180 and -90 <= y <= 90:
                    pixel_x = int(((x + 180.0) / 360.0) * width)
                    pixel_y = int(((90.0 - y) / 180.0) * height)
                    pixel_x = max(0, min(width - 1, pixel_x))
                    pixel_y = max(0, min(height - 1, pixel_y))
                    hole_coords.append((pixel_x, pixel_y))
            
            if len(hole_coords) >= 3:
                fill_polygon_scanline(img_array, id_array, hole_coords, (0, 0, 0), 0, width, height)

def fill_polygon_scanline(img_array, id_array, coords, color, province_id, width, height):
    """
    Algorithme de scan-line pour remplir un polygone pixel par pixel
    """
    if len(coords) < 3:
        return
    
    # Trouver les limites Y
    min_y = max(0, min(coord[1] for coord in coords))
    max_y = min(height - 1, max(coord[1] for coord in coords))
    
    # Pour chaque ligne de scan
    for y in range(min_y, max_y + 1):
        # Trouver les intersections avec les arêtes
        intersections = []
        
        for i in range(len(coords)):
            x1, y1 = coords[i]
            x2, y2 = coords[(i + 1) % len(coords)]
            
            # Vérifier si l'arête croise cette ligne
            if (y1 <= y < y2) or (y2 <= y < y1):
                if y2 != y1:  # Éviter division par zéro
                    x_intersect = x1 + (y - y1) * (x2 - x1) / (y2 - y1)
                    intersections.append(int(round(x_intersect)))
        
        # Trier les intersections
        intersections.sort()
        
        # Remplir entre les paires d'intersections
        for i in range(0, len(intersections), 2):
            if i + 1 < len(intersections):
                x_start = max(0, intersections[i])
                x_end = min(width - 1, intersections[i + 1])
                
                # Remplir pixel par pixel
                for x in range(x_start, x_end + 1):
                    # Éviter d'écraser des provinces déjà dessinées
                    if id_array[y, x] == 0 or id_array[y, x] == province_id:
                        img_array[y, x] = color
                        id_array[y, x] = province_id

def verify_pixel_perfect_colors(image_path, expected_colors):
    """
    Vérifier que chaque couleur attendue existe exactement dans l'image
    """
    print("\n=== VÉRIFICATION PIXEL-PARFAITE ===")
    
    img = Image.open(image_path)
    img_array = np.array(img)
    
    # Créer un dictionnaire des couleurs présentes
    unique_colors = {}
    for y in range(img_array.shape[0]):
        for x in range(img_array.shape[1]):
            color = tuple(img_array[y, x])
            if color in unique_colors:
                unique_colors[color] += 1
            else:
                unique_colors[color] = 1
    
    print(f"Couleurs uniques trouvées: {len(unique_colors)}")
    
    # Vérifier les couleurs attendues
    missing_colors = []
    for province_key, expected_color in expected_colors.items():
        if expected_color not in unique_colors:
            missing_colors.append((province_key, expected_color))
    
    if missing_colors:
        print(f"ERREUR: {len(missing_colors)} couleurs manquantes")
        for province_key, color in missing_colors[:5]:
            print(f"  {province_key}: RGB{color}")
    else:
        print("✓ Toutes les couleurs attendues sont présentes")
    
    # Vérifier s'il y a des couleurs "sales" (non exactes)
    dirty_colors = []
    expected_set = set(expected_colors.values())
    expected_set.add((0, 0, 0))  # Noir pour océan
    
    for color in unique_colors.keys():
        if color not in expected_set:
            dirty_colors.append(color)
    
    if dirty_colors:
        print(f"ATTENTION: {len(dirty_colors)} couleurs 'sales' détectées")
        for color in dirty_colors[:10]:
            print(f"  RGB{color} ({unique_colors[color]} pixels)")
        
        # Analyser les couleurs sales
        analyze_dirty_colors(dirty_colors, expected_set)
    else:
        print("✓ Aucune couleur 'sale' détectée")

def analyze_dirty_colors(dirty_colors, expected_colors):
    """
    Analyser les couleurs sales pour comprendre leur origine
    """
    print("\n=== ANALYSE DES COULEURS SALES ===")
    
    expected_list = list(expected_colors)
    
    for dirty_color in dirty_colors[:5]:  # Analyser les 5 premières
        # Trouver la couleur attendue la plus proche
        min_distance = float('inf')
        closest_expected = None
        
        for expected_color in expected_list:
            distance = sum(abs(a - b) for a, b in zip(dirty_color, expected_color))
            if distance < min_distance:
                min_distance = distance
                closest_expected = expected_color
        
        print(f"Couleur sale RGB{dirty_color} proche de RGB{closest_expected} (distance: {min_distance})")

def save_mappings_json(output_path, province_ids):
    """
    Sauvegarder les mappings en JSON
    """
    json_path = output_path.replace('.png', '_mapping.json')
    json_data = {
        'total_provinces': len(province_ids),
        'provinces': province_ids
    }
    
    with open(json_path, 'w', encoding='utf-8') as f:
        json.dump(json_data, f, indent=2, ensure_ascii=False)
    
    print(f"Mapping JSON sauvé : {json_path}")

# Utilisation
if __name__ == "__main__":
    shapefile_path = "ne_10m_admin_1_states_provinces.shp"
    output_path = "/home/PL/Documents/projets_Godot/World/world_project/assets/provinces/Province_Index_Map_Generated.png"
    
    import os
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    
    colors, ids = create_province_texture_pixel_perfect(shapefile_path, output_path)
    
    print(f"\n=== RÉSUMÉ ===")
    print(f"Provinces générées : {len(colors)}")
    print(f"Image générée : {output_path}")