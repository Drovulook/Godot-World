import json
import glob
from PIL import Image, ImageDraw

class JSONHandler:
    def __init__(self, ai_agent, web_scraper, print_bool = False, target_year = -500):
        self.ai_agent = ai_agent
        self.web_scraper = web_scraper

        self.print_bool = print_bool
        self.target_year = target_year
        self.data = {}

        self.important_place_types = {
            'polis', 'city', 'colonia', 'municipium', 
            'fortified-settlement', 'urban-center'
        }

        self.exclude_place_types = ['Unknown']

    def create_dictionary(self):
        """Trouve des lieux avec des dates spécifiées"""
        
        count = 0
        for json_file in glob.glob("data/json/**/*.json", recursive=True):
                
            try:
                with open(json_file, 'r', encoding='utf-8') as f:
                    data = json.load(f)
                
                name = data.get('title', 'Unknown')
                culture = data.get('subject', [])
                coor = data.get('reprPoint', [])
                description = data.get('description', '')

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

                if start and end and attestations and coor != []:
                    time_condition = start <= -500 and end >= -500
                    place_type_condition = not(place_type in self.exclude_place_types)
                    importance_condition = (culture != [] and "dare:major=1" in culture and "dare:ancient=1" in culture) or (place_type in self.important_place_types)
                    if (time_condition and place_type_condition and importance_condition):
                        count += 1
                        
                        color_val = 15
                        r = (count // (color_val * color_val)) % color_val * color_val
                        g = (count // color_val) % color_val * color_val           
                        b = count % color_val * color_val                  

                        if self.print_bool:
                            print(f"\nName: {name}, (ID: {count})")
                            print(f"  Start: {start}, End: {end}")
                            print(f"  Attestations: {len(attestations)}")
                            print(f"  Place Type: {place_type}")
                            print(f"  Culture Info: {', '.join(culture)}")
                            print(f"  Description: {description[:500]}...")
                            print(f"  Coordinates: {coor[0]}, {coor[1]}")
                            print(f"  Color: {'r': r, 'g': g, 'b': b}")
                            print("-" * 40 + "\n")

                        self.data[count] = {
                            'name': name,
                            'start': start,
                            'end': end,
                            'place_type': place_type,
                            'culture_info': culture,
                            'description': description,
                            'coordinates': coor,
                            'color': {'r': r, 'g': g, 'b': b}
                        }
                        
                                    
            except Exception as e:
                print(e)

        
        print(f"\nTotal places: {count}")

    def create_intermediate_data(self, output_path):
        print(f"\n=== CRÉATION DU FICHIER JSON ===")
        json_path = f"{output_path}intermediate_cities_result.json"
        with open(json_path, 'w', encoding='utf-8') as f:
            json.dump(self.data, f, indent=2, ensure_ascii=False)
        print("-> fichier json créé")

    def enrich_data_with_ai(self, input_file):
        with open(input_file, 'r', encoding='utf-8') as file:
            self.data = json.load(file)
        self.ai_agent.enrich_data(self.data)

    def create_enriched_output_data(self, output_path='/home/PL/Documents/projets_Godot/World/world_project/assets/cities/'):
        print(f"\n=== CRÉATION DU FICHIER JSON ===")
        json_path = f"{output_path}cities.json"
        with open(json_path, 'w', encoding='utf-8') as f:
            json.dump(self.data, f, indent=2, ensure_ascii=False)
        print("-> fichier json créé")

    
    def create_texture(self, output_path='/home/PL/Documents/projets_Godot/World/world_project/assets/cities/', 
                           texture_width=8640, texture_height=4320):
        print(f"\n=== CRÉATION DE LA TEXTURE ===")
        texture_path = f"{output_path}cities.png"

        if self.data == {}:
            with open(f"{output_path}cities.json", 'r', encoding='utf-8') as f:
                self.data = json.load(f)

        # Créer l'image
        img = Image.new('RGB', (texture_width, texture_height), color='black')
        draw = ImageDraw.Draw(img)

        for key, value in self.data.items():
            lon = value['coordinates'][0]
            lat = value['coordinates'][1]
            color_dict = value['color']
            color = (color_dict['r'], color_dict['g'], color_dict['b'])

            x = int((lon + 180) * texture_width / 360)
            y = int((90 - lat) * texture_height / 180)
            
            # S'assurer que les coordonnées sont dans l'image
            x = max(0, min(texture_width-1, x))
            y = max(0, min(texture_height-1, y))

            radius = 3
            draw.ellipse([x-radius, y-radius, x+radius, y+radius], 
                        fill=color, outline=color)

        img.save(texture_path)
        print("-> texture créée")