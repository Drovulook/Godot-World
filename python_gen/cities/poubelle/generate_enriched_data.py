import json
import glob
import numpy as np
from PIL import Image, ImageDraw
import colorsys
import matplotlib.pyplot as plt

class PleiadesData:
    def __init__(self, print_bool = False, target_year = -500, exclude_place_types=[]):
        self.print_bool = print_bool
        self.target_year = target_year
        self.exclude_place_types = exclude_place_types
        self.data = {}

    def create_dictionary(self):
        """Trouve des lieux avec des dates spécifiées"""
        
        count = 0
        for json_file in glob.glob("data/json/**/*.json", recursive=True):
                
            try:
                with open(json_file, 'r', encoding='utf-8') as f:
                    data = json.load(f)
                
                title = data.get('title', 'Unknown')
                culture = data.get('subject', [])
                coor = data.get('reprPoint', [])

                # Vérifier dates dans locations
                for location in data.get('locations', []):
                    start = location.get('start')
                    end = location.get('end')
                    attestations = location.get('attestations', [])

                # Vérifier snippet dans features
                place_type = "Unknown"
                for feature in data.get('features', []):
                    snippet = feature.get('properties', {}).get('snippet', '')
                    if "; " in snippet:
                        place_type = snippet.split(';')[0].strip()
                
                if start and end and attestations:
                    if (start <= -500 and end >= -500 and not(place_type in self.exclude_place_types)
                        and culture != [] and coor != [] and "dare:major=1" in culture and "dare:ancient=1" in culture):
                        count += 1
                        
                        id = data.get('id', 'Unknown')
                        color_val = 15
                        r = (count // (color_val * color_val)) % color_val * color_val
                        g = (count // color_val) % color_val * color_val           
                        b = count % color_val * color_val                  
                        
                        color = (r, g, b)

                        if self.print_bool:
                            print(f"\n{title} (ID: {id})")
                            print(f"  Start: {start}, End: {end}")
                            print(f"  Attestations: {len(attestations)}")
                            print(f"  Place Type: {place_type}")
                            print(f"  Culture: {', '.join(culture)}")
                            print(f"  Coordinates: {coor[0]}, {coor[1]}")
                            print(f"  Color: {color}")
                            print("-" * 40 + "\n")

                        self.data[id] = {
                            'title': title,
                            'id': id,
                            'start': start,
                            'end': end,
                            'place_type': place_type,
                            'culture': culture,
                            'coordinates': coor,
                            'color': color
                        }
                        
                                    
            except:
                continue

        
        print(f"\nTotal places: {count}")
    
    def create_output_data(self, output_path='/home/PL/Documents/projets_Godot/World/world_project/assets/cities/', 
                           texture_width=8640, texture_height=4320):
        print(f"\n=== CRÉATION DU FICHIER JSON ===")
        json_path = f"{output_path}cities.json"
        with open(json_path, 'w', encoding='utf-8') as f:
            json.dump(self.data, f, indent=2, ensure_ascii=False)
        print("-> fichier json créé")
    
        print(f"\n=== CRÉATION DE LA TEXTURE ===")
        texture_path = f"{output_path}cities.png"

        # Créer l'image
        img = Image.new('RGB', (texture_width, texture_height), color='black')
        draw = ImageDraw.Draw(img)

        for key, value in self.data.items():
            lon = value['coordinates'][0]
            lat = value['coordinates'][1]
            color = value['color']

            x = int((lon + 180) * texture_width / 360)
            y = int((90 - lat) * texture_height / 180)
            
            # S'assurer que les coordonnées sont dans l'image
            x = max(0, min(texture_width-1, x))
            y = max(0, min(texture_height-1, y))

            radius = 4
            draw.ellipse([x-radius, y-radius, x+radius, y+radius], 
                        fill=color, outline=color)

        img.save(texture_path)
        print("-> texture créée")

if __name__ == "__main__":
    pleiades_data = PleiadesData(print_bool=True)
    pleiades_data.create_dictionary()
    pleiades_data.create_output_data()