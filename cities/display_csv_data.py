import pandas as pd
import numpy as np
from PIL import Image, ImageDraw
import json
import colorsys
import matplotlib.pyplot as plt

class PleiadesGISAnalyzer:
    def __init__(self):
        self.places = None
        self.locations = None
        self.names = None
        self.place_types = None
        self.places_place_types = None
        
    def load_all_data(self):
        """Charge tous les fichiers CSV disponibles"""
        print("Chargement des données GIS...")
        
        try:
            # Fichiers principaux
            self.places = pd.read_csv('data/gis/places.csv')
            print(f"✓ Places: {len(self.places)} entrées")
            
            self.locations = pd.read_csv('data/gis/locations_points.csv')
            print(f"✓ Locations: {len(self.locations)} entrées")
            
            self.names = pd.read_csv('data/gis/names.csv')
            print(f"✓ Names: {len(self.names)} entrées")
            
            self.place_types = pd.read_csv('data/gis/place_types.csv')
            print(f"✓ Place types: {len(self.place_types)} types")
            
            self.places_place_types = pd.read_csv('data/gis/places_place_types.csv')
            print(f"✓ Places-types relations: {len(self.places_place_types)} entrées")
            
        except FileNotFoundError as e:
            print(f"Fichier manquant: {e}")
    
    def filter_places_by_period(self, target_year=-500, place_types_filter=None):
        """Filtre les lieux par période et types"""
        print(f"\n=== FILTRAGE POUR L'ANNÉE {target_year} ===")
        
        # 1. Filtrer les noms par période
        names_in_period = self.names[
            (self.names['year_after_which'] <= target_year) & 
            (self.names['year_before_which'] >= target_year)
        ]
        
        print(f"Noms actifs en {target_year}: {len(names_in_period)}")
        
        # 2. Récupérer les place_id uniques des noms actifs
        active_place_ids = names_in_period['place_id'].unique()
        print(f"Lieux uniques actifs: {len(active_place_ids)}")
        
        # 3. Filtrer les places correspondantes
        places_filtered = self.places[
            (self.places['id'].isin(active_place_ids)) &
            (self.places['representative_longitude'].notna()) &
            (self.places['representative_latitude'].notna())
        ].copy()
        
        print(f"Lieux avec coordonnées: {len(places_filtered)}")
        
        # 4. Ajouter les types de lieux
        places_with_types = places_filtered.merge(
            self.places_place_types, 
            left_on='id', 
            right_on='place_id',
            how='left'
        )
        
        # 5. Filtrer par types si spécifié
        if place_types_filter:
            places_with_types = places_with_types[
                places_with_types['place_type'].isin(place_types_filter)
            ]
            print(f"Après filtrage par types {place_types_filter}: {len(places_with_types)}")
        
        return places_with_types

    def get_major_cities(self, target_year=-500, min_importance=None):
        """Récupère les villes importantes"""
        print(f"\n=== VILLES IMPORTANTES EN {target_year} ===")
        
        # Types de lieux urbains
        urban_types = ['settlement', 'polis', 'city-center', 'urban']
        
        cities = self.filter_places_by_period(target_year, urban_types)
        
        # Filtrer par importance si spécifié
        if min_importance:
            cities = cities[cities.get('dare:major', 0) >= min_importance]
        
        # Grouper par lieu (un lieu peut avoir plusieurs types)
        cities_unique = cities.groupby('id').first().reset_index()
        
        print(f"Villes trouvées: {len(cities_unique)}")
        
        return cities_unique
    
    def create_world_texture(self, places_df, output_name="ancient_world", 
                           width=4096, height=2048):
        """Crée une texture du monde avec les lieux"""
        print(f"\n=== CRÉATION DE LA TEXTURE {output_name} ===")
        
        # Créer l'image
        img = Image.new('RGB', (width, height), color='black')
        draw = ImageDraw.Draw(img)
        
        # Préparer les données
        colors = []
        mapping = {
            "metadata": {
                "total_places": len(places_df),
                "image_size": [width, height],
                "projection": "equirectangular"
            },
            "places": []
        }
        
        for i, (_, place) in enumerate(places_df.iterrows()):
            # Coordonnées pixel
            lon = place['representative_longitude']
            lat = place['representative_latitude']
            
            x = int((lon + 180) * width / 360)
            y = int((90 - lat) * height / 180)
            
            # S'assurer que les coordonnées sont dans l'image
            x = max(0, min(width-1, x))
            y = max(0, min(height-1, y))
            
            # Couleur unique
            hue = (i * 137.508) % 360
            saturation = 0.7 + (i % 3) * 0.1
            value = 0.8 + (i % 2) * 0.2
            
            rgb = colorsys.hsv_to_rgb(hue/360, saturation, value)
            color = tuple(int(c * 255) for c in rgb)
            
            # Dessiner le point
            radius = 4 if place.get('dare:major', 0) == 1 else 2
            draw.ellipse([x-radius, y-radius, x+radius, y+radius], 
                        fill=color, outline=color)
            
            # Ajouter au mapping
            place_entry = {
                "id": int(place['id']),
                "title": place['title'],
                "coordinates": {
                    "longitude": float(lon),
                    "latitude": float(lat),
                    "pixel_x": x,
                    "pixel_y": y
                },
                "color": {
                    "rgb": color,
                    "hex": f"#{color[0]:02x}{color[1]:02x}{color[2]:02x}"
                },
                "place_type": place.get('place_type', 'unknown'),
                "period": f"{place.get('year_after_which', '?')} to {place.get('year_before_which', '?')}",
                "precision": place.get('location_precision', 'unknown'),
                "major": bool(place.get('dare:major', 0))
            }
            
            mapping["places"].append(place_entry)
        
        # Sauvegarder
        img_path = f"{output_name}.png"
        json_path = f"{output_name}_mapping.json"
        
        img.save(img_path)
        
        with open(json_path, 'w', encoding='utf-8') as f:
            json.dump(mapping, f, indent=2, ensure_ascii=False)
        
        print(f"✓ Image: {img_path}")
        print(f"✓ Mapping: {json_path}")
        
        return img_path, json_path
    
    def analyze_by_region(self, target_year=-500):
        """Analyse par région géographique"""
        places = self.filter_places_by_period(target_year)
        
        # Définir des régions approximatives
        regions = {
            "Mediterranean": (-10, 30, 45, 45),  # lon_min, lat_min, lon_max, lat_max
            "Northern Europe": (-10, 45, 30, 70),
            "Middle East": (30, 25, 65, 45),
            "North Africa": (-10, 15, 40, 35)
        }
        
        print(f"\n=== ANALYSE PAR RÉGION ({target_year}) ===")
        
        for region_name, (lon_min, lat_min, lon_max, lat_max) in regions.items():
            region_places = places[
                (places['representative_longitude'] >= lon_min) &
                (places['representative_longitude'] <= lon_max) &
                (places['representative_latitude'] >= lat_min) &
                (places['representative_latitude'] <= lat_max)
            ]
            
            print(f"{region_name}: {len(region_places)} lieux")
            
            if len(region_places) > 0:
                top_types = region_places['place_type'].value_counts().head(3)
                print(f"  Types principaux: {dict(top_types)}")
    
    def export_specific_data(self, target_year=-500, place_types=None, 
                           regions=None, output_format='csv'):
        """Exporte des données spécifiques selon critères"""
        places = self.filter_places_by_period(target_year, place_types)
        
        # Filtrer par régions si spécifié
        if regions:
            # Appliquer les filtres régionaux
            pass
        
        # Ajouter les noms alternatifs
        places_with_names = places.merge(
            self.names[['place_id', 'romanized_form_1', 'attested_form', 'language_tag']], 
            left_on='id', 
            right_on='place_id',
            how='left'
        )
        
        if output_format == 'csv':
            filename = f"pleiades_export_{abs(target_year)}bc.csv"
            places_with_names.to_csv(filename, index=False)
            print(f"✓ Export CSV: {filename}")
        
        elif output_format == 'json':
            filename = f"pleiades_export_{abs(target_year)}bc.json"
            places_with_names.to_json(filename, orient='records', indent=2)
            print(f"✓ Export JSON: {filename}")
        
        return filename

# Utilisation du script
def main():
    analyzer = PleiadesGISAnalyzer()
    
    # 1. Charger toutes les données
    analyzer.load_all_data()
    
    # 2. Récupérer les villes importantes en -500
    cities_500bc = analyzer.get_major_cities(target_year=-500)
    
    # 3. Créer une texture pour -500
    analyzer.create_world_texture(cities_500bc, "cities_500bc")
    
    # 4. Analyse par région
    analyzer.analyze_by_region(-500)
    
    # 5. Export de données spécifiques
    analyzer.export_specific_data(
        target_year=-500,
        place_types=['settlement', 'polis', 'temple'],
        output_format='csv'
    )
    
    # 7. Comparer différentes périodes
    print("\n=== COMPARAISON TEMPORELLE ===")
    for year in [-500, -100, 100, 400]:
        places = analyzer.filter_places_by_period(year)
        print(f"Année {year}: {len(places)} lieux actifs")

if __name__ == "__main__":
    main()