import pandas as pd
import numpy as np
from PIL import Image, ImageDraw
import json
import colorsys
import matplotlib.pyplot as plt

class PleiadesData:
    def __init__(self):
        self.places = None
        self.locations = None
        self.names = None
        self.place_types = None
        self.places_place_types = None
    
        self.all_data = None
        
    def load_all_data(self):
        """Charge tous les fichiers CSV disponibles"""
        print("Chargement des données GIS...")
        
        try:
            # Fichiers principaux
            folder_path = 'data/csv/'

            self.places = pd.read_csv(folder_path + 'places.csv')
            print(f"✓ Places: {len(self.places)} entrées")
            
            self.locations = pd.read_csv(folder_path + 'location_points.csv')
            print(f"✓ Locations: {len(self.locations)} entrées")
            
            self.names = pd.read_csv(folder_path + 'names.csv')
            print(f"✓ Names: {len(self.names)} entrées")

            self.place_types = pd.read_csv(folder_path + 'place_types.csv')
            print(f"✓ Place types: {len(self.place_types)} types")
            
            self.places_place_types = pd.read_csv(folder_path + 'places_place_types.csv')
            print(f"✓ Places-types relations: {len(self.places_place_types)} entrées")
            
            # créer self.data en combinant les données
            names_columns = ['place_id', 'title', 'description', 'language_tag', 'year_after_which', 'year_before_which']

            self.data = self.names[names_columns].copy().rename(columns={'place_id': 'id'})
            self.data = self.data.merge(
                self.places[['id', 'representative_latitude','representative_longitude']],
                left_on='id', 
                right_on='id',
                how='left'
                ).merge(
                    self.places_place_types.rename(columns={'place_id': 'id'}),
                    left_on='id',
                    right_on='id',
                    how='left'
                )

            print("data columns: ", self.data.columns.tolist())

        except FileNotFoundError as e:
            print(f"Fichier manquant: {e}")
    
    def filter(self, target_year, exclude_place_types=None):

        # pas de valeur nulle
        self.data = self.data[
            (self.data['year_before_which'].notna()) &
            (self.data['year_after_which'].notna()) &
            (self.data['representative_longitude'].notna()) &
            (self.data['representative_latitude'].notna())
        ]

        self.data = self.data[
            (self.data['year_after_which'] <= target_year) &
            (self.data['year_before_which'] >= target_year)
        ]

        if exclude_place_types:
            self.data = self.data[
                ~self.data['place_type'].isin(exclude_place_types)
            ]

        print("\n")
        print("data after filter: ", self.data.iloc[0:20, 0:])
        print(f"Nombre de lieux après filtrage: {len(self.data)}")

def main():
    data = PleiadesData()
    data.load_all_data()
    data.filter(target_year= -500, exclude_place_types=['settlement', 'cemetery', 'mountain', 'archaeological-site', 'region', 'country', 'continent', 'mountain range'])

if __name__ == "__main__":
    main()