from PIL import Image
import os
import gc

# Augmenter la limite de pixels autorisés par PIL
Image.MAX_IMAGE_PIXELS = None  # Désactive complètement la limite

def convert_jpg_to_png_safe(input_path, output_path):
    """
    Convertit une très grande image JPG en PNG avec gestion sécurisée de la mémoire
    """
    try:
        print(f"Conversion de {input_path} vers {output_path}")
        
        # Vérifier si le fichier existe
        if not os.path.exists(input_path):
            print(f"Erreur: Le fichier {input_path} n'existe pas")
            return False
            
        # Obtenir la taille du fichier
        file_size = os.path.getsize(input_path)
        print(f"Taille du fichier: {file_size / (1024*1024):.1f} MB")
        
        # Ouvrir l'image avec précaution
        print("Ouverture de l'image...")
        with Image.open(input_path) as img:
            width, height = img.size
            print(f"Dimensions: {width}x{height} pixels")
            print(f"Total pixels: {width * height:,}")
            print(f"Mode couleur: {img.mode}")
            
            # Convertir en RGB si nécessaire
            if img.mode != 'RGB':
                print("Conversion en mode RGB...")
                img = img.convert('RGB')
            
            # Pour les très grandes images, traiter par chunks plus petits
            chunk_height = 128  # Réduire la taille des chunks
            print(f"Traitement par bandes de {chunk_height} pixels...")
            
            # Créer l'image de sortie
            result = Image.new('RGB', (width, height))
            
            # Traiter l'image par bandes
            total_chunks = (height + chunk_height - 1) // chunk_height
            
            for chunk_idx in range(total_chunks):
                y_start = chunk_idx * chunk_height
                y_end = min(y_start + chunk_height, height)
                current_height = y_end - y_start
                
                print(f"Chunk {chunk_idx + 1}/{total_chunks}: lignes {y_start}-{y_end}")
                
                # Extraire et traiter le chunk
                chunk = img.crop((0, y_start, width, y_end))
                result.paste(chunk, (0, y_start))
                
                # Libérer la mémoire du chunk
                del chunk
                gc.collect()
                
                # Afficher le progrès
                progress = ((chunk_idx + 1) / total_chunks) * 100
                print(f"Progrès: {progress:.1f}%")
            
            # Sauvegarder avec compression optimisée
            print("Sauvegarde en cours...")
            result.save(output_path, 'PNG', optimize=True, compress_level=6)
            
            print("✅ Conversion réussie!")
            
            # Vérifier la taille du fichier de sortie
            output_size = os.path.getsize(output_path)
            print(f"Fichier PNG créé: {output_size / (1024*1024):.1f} MB")
            
            return True
            
    except MemoryError:
        print("❌ Erreur: Mémoire insuffisante")
        return False
    except Exception as e:
        print(f"❌ Erreur lors de la conversion: {e}")
        return False

def convert_with_resize(input_path, output_path, max_width=4096):
    """
    Alternative: convertir en réduisant la taille si nécessaire
    """
    try:
        print(f"Conversion avec redimensionnement si nécessaire...")
        
        with Image.open(input_path) as img:
            width, height = img.size
            print(f"Dimensions originales: {width}x{height}")
            
            # Calculer les nouvelles dimensions si redimensionnement nécessaire
            if width > max_width:
                ratio = max_width / width
                new_width = max_width
                new_height = int(height * ratio)
                print(f"Redimensionnement vers: {new_width}x{new_height}")
                
                # Redimensionner avec un algorithme de haute qualité
                img = img.resize((new_width, new_height), Image.Resampling.LANCZOS)
            
            # Convertir en RGB si nécessaire
            if img.mode != 'RGB':
                img = img.convert('RGB')
            
            # Sauvegarder
            img.save(output_path, 'PNG', optimize=True, compress_level=6)
            
            print("✅ Conversion avec redimensionnement réussie!")
            return True
            
    except Exception as e:
        print(f"❌ Erreur lors de la conversion avec redimensionnement: {e}")
        return False

def main():
    input_file = "/home/PL/Documents/projets_Godot/World/images/earth_complete.jpg"
    output_file = "/home/PL/Documents/projets_Godot/World/images/earth_complete.png"
    output_file_resized = "/home/PL/Documents/projets_Godot/World/images/earth_complete_4k.png"
    
    # Créer le répertoire de sortie si nécessaire
    os.makedirs(os.path.dirname(output_file), exist_ok=True)
    
    print("=== Option 1: Conversion sans redimensionnement ===")
    success = convert_jpg_to_png_safe(input_file, output_file)
    
    if not success:
        print("\n=== Option 2: Conversion avec redimensionnement à 4K ===")
        success = convert_with_resize(input_file, output_file_resized, max_width=4096)
        
        if success:
            # Mettre à jour le chemin dans votre code C++
            print(f"\n📝 Modifiez votre code C++ pour utiliser:")
            print(f'std::string texture_path_abs = "{output_file_resized}";')
        else:
            print("\n=== Option 3: Conversion avec redimensionnement à 2K ===")
            output_file_2k = "/home/PL/Documents/projets_Godot/World/images/earth_complete_2k.png"
            success = convert_with_resize(input_file, output_file_2k, max_width=2048)
            
            if success:
                print(f"\n📝 Modifiez votre code C++ pour utiliser:")
                print(f'std::string texture_path_abs = "{output_file_2k}";')

if __name__ == "__main__":
    main()
