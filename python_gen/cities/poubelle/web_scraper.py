import requests
import time
from typing import Dict, Any, List
import copy
from bs4 import BeautifulSoup
from urllib.parse import quote_plus
import re
from urllib.parse import unquote

class WebScraper:
    def __init__(self):
        self.session = requests.Session()
        self.session.headers.update({
            'User-Agent': 'Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36'
        })
    
    ########################################################################

    def search_web(self, city_name: str) -> List[str]:
        try:
            search_terms = f"{city_name} ancient city history archaeology"
            search_query = quote_plus(search_terms)
            search_url = f"https://duckduckgo.com/html/?q={search_query}" 
            
            print(f"🔍 Recherche web pour: {city_name}")
            response = self.session.get(search_url, timeout=10)
            response.raise_for_status()
            soup = BeautifulSoup(response.content, 'html.parser')
                
            # Extraire les liens des résultats
            links = []
            print(soup)
            for result in soup.find_all('a', class_='result__a')[:3]:  # Top 3 résultats
                href = result.get('href')
                if href and not href.startswith('/'):
                    # Nettoyer l'URL DuckDuckGo
                    if 'uddg=' in href:
                        actual_url = href.split('uddg=')[1].split('&')[0]
                        actual_url = unquote(actual_url)
                        links.append(actual_url)
            
            if not links:
                print("❌ Aucun lien trouvé")
                return ""
            
            # Extraire le contenu du premier site pertinent
            for link in links:
                content = self.extract_content(link)
                if content and len(content) > 200:  # Au moins 200 caractères
                    print(f"✅ Contenu extrait de: {link[:50]}...")
                    return content[:2000]  # Limiter à 2000 caractères
            
            return ""
        
        except Exception as e:
            print(f"❌ Erreur lors de la recherche web: {e}")
            return ""

    ########################################################################

    def extract_content(self, url: str) -> str:
        """Extrait et nettoie le contenu HTML d'une page"""
        try:
            response = self.session.get(url, timeout=10)
            response.raise_for_status()
            
            soup = BeautifulSoup(response.content, 'html.parser')
            
            # Supprimer les éléments indésirables
            for element in soup(['script', 'style', 'nav', 'header', 'footer', 'aside', 'advertisement']):
                element.decompose()
            
            # Chercher le contenu principal
            content_selectors = [
                'article', 'main', '.content', '#content', 
                '.post-content', '.entry-content', '.article-content'
            ]
            
            content = ""
            for selector in content_selectors:
                element = soup.select_one(selector)
                if element:
                    content = element.get_text(separator=' ', strip=True)
                    break
            
            # Si pas de contenu principal trouvé, prendre le body
            if not content:
                body = soup.find('body')
                if body:
                    content = body.get_text(separator=' ', strip=True)
            
            # Nettoyer le texte
            content = re.sub(r'\s+', ' ', content)  # Normaliser les espaces
            content = re.sub(r'\n+', '\n', content)  # Normaliser les retours à la ligne
            
            return content.strip()
            
        except Exception as e:
            print(f"❌ Erreur extraction contenu de {url}: {e}")
            return ""

if __name__ == "__main__":
    scraper = WebScraper()
    city_name = "Gyaur Kala"
    content = scraper.search_web(city_name)
