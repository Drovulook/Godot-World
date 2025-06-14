import geopandas as gpd
import matplotlib.pyplot as plt
import numpy as np
from PIL import Image, ImageDraw
import matplotlib.patches as patches

def create_province_texture(shapefile_path, output_path, width=8640, height=4320):
    """
    Créer une texture où chaque province/état a une couleur unique
    """
    
    # Charger les données Natural Earth
    world = gpd.read_file(shapefile_path)
    
    print(f"Nombre de provinces/états : {len(world)}")
    print("Colonnes disponibles :", world.columns.tolist())
    
    # Afficher quelques exemples pour comprendre la structure
    print("\nExemples de données :")
    for idx, row in world.head(10).iterrows():
        # CORRECTION : Utiliser les vrais noms de colonnes (en minuscules)
        country = row.get('adm0_a3', 'Unknown')  # ← Changé de 'ADM0_A3' à 'adm0_a3'
        province = row.get('name', 'Unknown')    # ← Déjà correct
        admin_level = row.get('type_en', 'Unknown')  # ← Changé de 'TYPE' à 'type_en'
        country_name = row.get('admin', 'Unknown')   # ← Changé de 'ADMIN' à 'admin'
        print(f"  Pays: {country} ({country_name}), Province: {province}, Type: {admin_level}")
    
    # Créer une image RGB
    img = Image.new('RGB', (width, height), (0, 0, 0))  # Noir pour l'océan
    draw = ImageDraw.Draw(img)
    
    # Organiser par pays et générer des couleurs
    province_colors = {}
    province_ids = {}
    countries = {}
    
    # Grouper par pays
    for idx, (_, province) in enumerate(world.iterrows()):
        # CORRECTION : Utiliser les vrais noms de colonnes
        country_code = province.get('adm0_a3', 'UNK')     # ← Changé
        country_name = province.get('admin', 'Unknown')   # ← Changé
        province_name = province.get('name', f'Province_{idx}')
        province_type = province.get('type_en', 'Province')  # ← Changé
        
        # Debug : afficher les premières valeurs pour vérifier
        if idx < 5:
            print(f"Debug {idx}: Country={country_code}, Name={country_name}, Province={province_name}, Type={province_type}")
        
        if country_code not in countries:
            countries[country_code] = {
                'name': country_name,
                'provinces': []
            }
        
        countries[country_code]['provinces'].append({
            'index': idx,
            'name': province_name,
            'type': province_type,
            'geometry': province.geometry
        })
    
    # Générer des couleurs par pays et provinces
    country_id = 0
    global_province_id = 1  # Commencer à 1 pour éviter le noir
    
    for country_code, country_data in countries.items():
        country_id += 1
        print(f"\n=== {country_data['name']} ({country_code}) - {len(country_data['provinces'])} provinces ===")
        
        for prov_idx, province_data in enumerate(country_data['provinces']):
            # Générer un ID couleur unique globalement
            province_id = global_province_id
            global_province_id += 1
            
            # Convertir l'ID en couleur RGB (24 bits)
            r = (province_id >> 16) & 0xFF
            g = (province_id >> 8) & 0xFF  
            b = province_id & 0xFF
            
            # Éviter le noir complet (réservé à l'océan)
            if r == 0 and g == 0 and b == 0:
                r = 1
            
            color = (r, g, b)
            
            # Créer une clé unique pour la province
            province_key = f"{country_code}_{province_data['name']}"
            
            province_colors[province_key] = color
            province_ids[province_key] = {
                'global_id': province_id,
                'country_code': country_code,
                'country_name': country_data['name'],
                'province_name': province_data['name'],
                'province_type': province_data['type']
            }
            
            print(f"  {province_data['name']} ({province_data['type']}) -> ID: {province_id} -> RGB: {color}")
    
    print(f"\nTotal : {len(province_colors)} provinces dans {len(countries)} pays")
    
    # Dessiner les provinces
    for province_key, color in province_colors.items():
        province_info = province_ids[province_key]
        country_code = province_info['country_code']
        
        # Trouver la géométrie correspondante
        province_geom = None
        for country_data in countries.values():
            for prov_data in country_data['provinces']:
                if f"{country_code}_{prov_data['name']}" == province_key:
                    province_geom = prov_data['geometry']
                    break
            if province_geom:
                break
        
        if province_geom is None or province_geom.is_empty:
            continue
            
        # Convertir la géométrie en coordonnées pixel
        if province_geom.geom_type == 'Polygon':
            polygons = [province_geom]
        elif province_geom.geom_type == 'MultiPolygon':
            polygons = list(province_geom.geoms)
        else:
            continue
            
        for polygon in polygons:
            try:
                # Convertir les coordonnées géographiques en coordonnées pixel
                exterior_coords = []
                for x, y in polygon.exterior.coords:
                    # Projection équirectangulaire simple
                    pixel_x = int((x + 180) / 360 * width)
                    pixel_y = int((90 - y) / 180 * height)
                    
                    # Clamper les valeurs
                    pixel_x = max(0, min(width - 1, pixel_x))
                    pixel_y = max(0, min(height - 1, pixel_y))
                    
                    exterior_coords.append((pixel_x, pixel_y))
                
                if len(exterior_coords) > 2:
                    draw.polygon(exterior_coords, fill=color, outline=color)
                    
                # Traiter les trous (îles intérieures)
                for interior in polygon.interiors:
                    hole_coords = []
                    for x, y in interior.coords:
                        pixel_x = int((x + 180) / 360 * width)
                        pixel_y = int((90 - y) / 180 * height)
                        pixel_x = max(0, min(width - 1, pixel_x))
                        pixel_y = max(0, min(height - 1, pixel_y))
                        hole_coords.append((pixel_x, pixel_y))
                    
                    if len(hole_coords) > 2:
                        draw.polygon(hole_coords, fill=(0, 0, 0))  # Noir pour les trous
            except Exception as e:
                print(f"Erreur lors du dessin de {province_key}: {e}")
                continue
    
    # Sauvegarder l'image
    img.save(output_path)
    print(f"Texture sauvegardée : {output_path}")
    
    # Sauvegarder la correspondance couleur -> province (format détaillé)
    mapping_path = output_path.replace('.png', '_mapping.txt')
    with open(mapping_path, 'w', encoding='utf-8') as f:
        f.write("Country_Code\tCountry_Name\tProvince_Name\tProvince_Type\tGlobal_ID\tR\tG\tB\n")
        for province_key, color in province_colors.items():
            info = province_ids[province_key]
            f.write(f"{info['country_code']}\t{info['country_name']}\t{info['province_name']}\t{info['province_type']}\t{info['global_id']}\t{color[0]}\t{color[1]}\t{color[2]}\n")
    
    # Sauvegarder aussi un fichier JSON pour faciliter l'usage programmatique
    import json
    json_path = output_path.replace('.png', '_mapping.json')
    json_data = {
        'total_provinces': len(province_colors),
        'total_countries': len(countries),
        'provinces': {}
    }
    
    for province_key, color in province_colors.items():
        info = province_ids[province_key]
        json_data['provinces'][str(info['global_id'])] = {
            'country_code': info['country_code'],
            'country_name': info['country_name'],
            'province_name': info['province_name'],
            'province_type': info['province_type'],
            'color': {'r': color[0], 'g': color[1], 'b': color[2]}
        }
    
    with open(json_path, 'w', encoding='utf-8') as f:
        json.dump(json_data, f, indent=2, ensure_ascii=False)
    
    print(f"Mapping TXT sauvé : {mapping_path}")
    print(f"Mapping JSON sauvé : {json_path}")
    
    return province_colors, province_ids

# Utilisation
if __name__ == "__main__":
    shapefile_path = "ne_10m_admin_1_states_provinces.shp"
    output_path = "/home/PL/Documents/projets_Godot/World/world_project/assets/provinces/Province_Index_Map_Generated.png"
    
    # Créer le dossier de sortie s'il n'existe pas
    import os
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    
    colors, ids = create_province_texture(shapefile_path, output_path)
    
    print(f"\n=== RÉSUMÉ ===")
    print(f"Provinces générées : {len(colors)}")
    print(f"Fichiers créés :")
    print(f"  - Texture : {output_path}")
    print(f"  - Mapping TXT : {output_path.replace('.png', '_mapping.txt')}")
    print(f"  - Mapping JSON : {output_path.replace('.png', '_mapping.json')}")