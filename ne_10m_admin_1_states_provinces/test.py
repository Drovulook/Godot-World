import geopandas as gpd

# Charger et examiner le shapefile
gdf = gpd.read_file("ne_10m_admin_1_states_provinces.shp")
print("Colonnes disponibles :")
print(gdf.columns.tolist())
print("\nPremières lignes :")
print(gdf.head())
print(f"\nNombre d'entités : {len(gdf)}")