import xarray as xr
import numpy as np

# Remplace "ton_fichier.nc" par le chemin de ton fichier
fichier_nc = "GEBCO_2024_TID.nc"

# Ouvrir le fichier NetCDF
ds = xr.open_dataset(fichier_nc)

# Afficher les dimensions, variables, attributs
print("=== Aperçu du Dataset ===")
print(ds)

# Afficher les noms des variables contenues
print("\n=== Variables disponibles ===")
print(list(ds.variables))

print("\n=== Values ===")

vals_uniques = np.unique(ds['tid'].values[:100])
print("Valeurs uniques dans 'tid' :", vals_uniques)