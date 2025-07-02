import geopandas as gpd
import matplotlib.pyplot as plt
import numpy as np
from PIL import Image, ImageDraw
import matplotlib.patches as patches

def create_country_texture(shapefile_path, output_path, width=4320, height=2160):
    """
    Créer une texture où chaque pays a une couleur unique
    """
    
    # Charger les données Natural Earth
    world = gpd.read_file(shapefile_path)
    
    print(f"Nombre de pays/territoires : {len(world)}")
    print("Colonnes disponibles :", world.columns.tolist())
    
    # Créer une image RGB
    img = Image.new('RGB', (width, height), (0, 0, 0))  # Noir pour l'océan
    draw = ImageDraw.Draw(img)
    
    # Générer une couleur unique pour chaque pays
    country_colors = {}
    country_ids = {}
    
    for idx, (_, country) in enumerate(world.iterrows()):
        # Générer un ID couleur unique (éviter le noir pour l'océan)
        country_id = idx + 1
        
        # Convertir l'ID en couleur RGB (24 bits = 16M de couleurs possibles)
        r = (country_id >> 16) & 0xFF
        g = (country_id >> 8) & 0xFF  
        b = country_id & 0xFF
        
        # Éviter le noir complet
        if r == 0 and g == 0 and b == 0:
            r = 1
        
        color = (r, g, b)
        country_name = country.get('NAME', f'Country_{idx}')
        
        country_colors[country_name] = color
        country_ids[country_name] = country_id
        
        print(f"Pays: {country_name} -> ID: {country_id} -> RGB: {color}")
    
    # Convertir les géométries en pixels et dessiner
    bounds = world.total_bounds  # [minx, miny, maxx, maxy]
    
    for idx, (_, country) in enumerate(world.iterrows()):
        geom = country.geometry
        country_name = country.get('NAME', f'Country_{idx}')
        color = country_colors[country_name]
        
        if geom is None:
            continue
            
        # Convertir la géométrie en coordonnées pixel
        if geom.geom_type == 'Polygon':
            polygons = [geom]
        elif geom.geom_type == 'MultiPolygon':
            polygons = list(geom.geoms)
        else:
            continue
            
        for polygon in polygons:
            # Convertir les coordonnées géographiques en coordonnées pixel
            exterior_coords = []
            for x, y in polygon.exterior.coords:
                # Normaliser en longitude/latitude
                pixel_x = int((x + 180) / 360 * width)
                pixel_y = int((90 - y) / 180 * height)
                
                # Clamper les valeurs
                pixel_x = max(0, min(width - 1, pixel_x))
                pixel_y = max(0, min(height - 1, pixel_y))
                
                exterior_coords.append((pixel_x, pixel_y))
            
            if len(exterior_coords) > 2:
                draw.polygon(exterior_coords, fill=color)
                
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
    
    # Sauvegarder l'image
    img.save(output_path)
    
    # Sauvegarder la correspondance couleur -> pays
    with open(output_path.replace('.png', '_mapping.txt'), 'w', encoding='utf-8') as f:
        f.write("Country_Name\tCountry_ID\tR\tG\tB\n")
        for name, color in country_colors.items():
            country_id = country_ids[name]
            f.write(f"{name}\t{country_id}\t{color[0]}\t{color[1]}\t{color[2]}\n")
    
    print(f"Texture créée : {output_path}")
    print(f"Mapping sauvé : {output_path.replace('.png', '_mapping.txt')}")
    
    return country_colors, country_ids

# Utilisation
if __name__ == "__main__":
    shapefile_path = "/home/PL/Documents/projets_Godot/World/ne_10m_admin_0_countries/ne_10m_admin_0_countries.shp"
    output_path = "/home/PL/Documents/projets_Godot/World/world_project/assets/Country_Index_Map_Generated.png"
    
    colors, ids = create_country_texture(shapefile_path, output_path)