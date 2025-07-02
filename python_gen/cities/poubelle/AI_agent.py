import json
import requests
import time
from typing import Dict, Any, List
import copy

class AiAgent:
    def __init__(self, ollama_url: str = "http://localhost:11434", model: str = "phi3:mini"):
        self.ollama_url = ollama_url
        self.model = model
        self.dict_len = 1
        self.current_city_index = 1 

    def create_prompt(self, value_dict):

    
        name = value_dict.get('name', 'Unknown City')
        place_type = value_dict.get('place_type', 'Unknown')
        description = value_dict.get('description', 'No description available')
        coordinates = value_dict.get('coordinates', [0, 0])
        lat, lon = coordinates
        return f"""
        For the city of {name}, which is a {place_type} located at coordinates {lat}, {lon} in the year 500 BC with the following description: "{description}", 
        provide me with the following information in strict JSON format:
        
        {{
            "dominant_culture": "name of the dominant culture/civilization",
            "estimated_population": estimated number of inhabitants,
            "political_importance": "local/regional/major",
            "economic_activities": ["activity1", "activity2", "activity3"],
            "main_religion": "dominant religion",
            "political_status": "city-state/colony/capital/village",
            "local_resources": ["resource1", "resource2"],
            "short_description": "3-4 sentence description of the city at that time using the provided description and your knowledge"
        }}

        IMPORTANT RULES:
        - For "estimated_population": provide ONLY a number (e.g., 15000, not "15,000 inhabitants"). No explanation.
        - For "dominant_culture": use specific civilization names at the time (e.g., "Greek", "Persian", "Egyptian"). Never use "unknown". No explanation. Use short names.
        - For "main_religion": use specific religion names at the time (do not use "polytheism", "monotheism" or "paganism"). Only use "unknown" if you have absolutely no historical information. No explanation. Use short names.
        - For "political_importance": use exactly one of: "local", "regional", "major". No explanation.
        - For "political_status": use exactly one of: "city-state", "colony", "capital", "village", "town". No explanation.
        
        Reply ONLY with valid JSON, no explanatory text before or after.
        """
    

    def query_ollama(self, prompt):
        try:
            response = requests.post(
                f"{self.ollama_url}/api/chat",
                json={
                "model": self.model,
                    "messages": [
                        {
                            "role": "user",
                            "content": prompt
                        }
                    ],
                    "stream": False
                },
                timeout=30
            )
            response.raise_for_status()
            return response.json().get("message", {}).get("content", "")
        except Exception as e:
            print(f"Erreur lors de la requête Ollama: {e}")
            return ""
    
    def parse_ollama_response(self, response):
        try:
            start = response.find('{')
            end = response.rfind('}') + 1
            if start != -1 and end != 0:
                json_str = response[start:end]
                return json.loads(json_str)
        except json.JSONDecodeError:
            pass

        return {
            "dominant_culture": "Unknown",
            "estimated_population": 5000,
            "political_importance": "local",
            "economic_activities": ["agriculture", "crafts"],
            "main_religion": "Unknown",
            "political_status": "village",
            "local_resources": ["arable land"],
            "short_description": "Historical information not available."
        }
    
    def enrich_city(self, value_dict):
        print("\n" + "-"* 60,f"\nville en cours de traitement: {value_dict.get('title', 'Unknown City')}")
        prompt = self.create_prompt(value_dict)
        response = self.query_ollama(prompt)

        if response:
            enriched_data = self.parse_ollama_response(response)
            # Fusionne les données existantes avec les nouvelles
            value_dict.update(enriched_data)
            print("✅ Enrichissement réussi!")
            print(f"📍 Nom: {value_dict.get('title', 'Unknown City')}")
            print(f"🏛️ Culture dominante: {value_dict.get('dominant_culture', 'N/A')}")
            print(f"👥 Population estimée: {value_dict.get('estimated_population', 'N/A')}")
            print(f"⚖️ Importance politique: {value_dict.get('political_importance', 'N/A')}")
            print(f"🏛️ Statut politique: {value_dict.get('political_status', 'N/A')}")
            print(f"🙏 Religion principale: {value_dict.get('main_religion', 'N/A')}")
        else:
            print(f"❌ Impossible d'enrichir {value_dict.get('title', 'Unknown City')}, données par défaut utilisées")
            value_dict.update(self.parse_ollama_response(""))
        
        print("\n", self.current_city_index, "/", self.dict_len, "villes traitées")
        self.current_city_index += 1
    
    def enrich_data(self, data):
        self.dict_len = len(data.keys())
        print("=" * 100)
        print(f"Traitement de {self.dict_len} villes par l'IA...")
        output_dict = copy.deepcopy(data)
        for key, value in output_dict.items():
            self.enrich_city(value)
