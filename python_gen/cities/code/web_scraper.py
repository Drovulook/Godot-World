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
        self.output_length = 2000

        self.session = requests.Session()
        self.session.headers.update({
            'User-Agent': 'Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36',
            'Accept': 'text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8',
            'Accept-Language': 'en-US,en;q=0.5',
            'Accept-Encoding': 'gzip, deflate',
            'Connection': 'keep-alive',
            'Upgrade-Insecure-Requests': '1',
        })
    
    ########################################################################

    def search_web_wikipedia(self, city_name: str) -> str:
        """Recherche directement sur Wikipedia"""
        try:
            # Recherche Wikipedia API
            wiki_search_url = "https://en.wikipedia.org/api/rest_v1/page/summary/"
            
            # Essayer différentes variantes du nom
            search_terms = [
                city_name,
                city_name.replace("-", " "),
                city_name.replace(" ", "_"),
                f"{city_name} archaeological site",
                f"{city_name} ancient city"
            ]
            
            for term in search_terms:
                try:
                    url = wiki_search_url + quote_plus(term)
                    response = self.session.get(url, timeout=10)
                    
                    if response.status_code == 200:
                        data = response.json()
                        if 'extract' in data and len(data['extract']) > 100:
                            print(f"✅ Trouvé sur Wikipedia: {term}")
                            return data['extract'][:self.output_length]
                except:
                    continue
            
            print("❌ Aucune page Wikipedia trouvée")
            return ""
            
        except Exception as e:
            print(f"❌ Erreur recherche Wikipedia: {e}")
            return ""


    def search_web_direct_sources(self, city_name: str) -> str:
        """Recherche directement sur des sites spécialisés"""
        try:
            # Sources spécialisées en archéologie/histoire
            sources = [
                f"https://en.wikipedia.org/wiki/{quote_plus(city_name)}",
                f"https://en.wikipedia.org/wiki/{quote_plus(city_name)}_archaeological_site",
                f"https://www.britannica.com/search?query={quote_plus(city_name)}",
                f"https://pleiades.stoa.org/search?SearchableText={quote_plus(city_name)}",
            ]
            
            for url in sources:
                try:
                    print(f"🔍 Tentative: {url}")
                    response = self.session.get(url, timeout=10)
                    
                    if response.status_code == 200:
                        content = self.extract_content_from_response(response)
                        if content and len(content) > 200:
                            print(f"✅ Contenu trouvé sur: {url[:50]}...")
                            return content[:self.output_length]
                except:
                    continue
            
            return ""
            
        except Exception as e:
            print(f"❌ Erreur recherche directe: {e}")
            return ""


    def search_web_google(self, city_name: str) -> str:
        """Recherche Google avec User-Agent plus sophistiqué"""
        try:
            search_terms = f"{city_name} ancient city history archaeology"
            search_query = quote_plus(search_terms)
            
            # Utiliser Google Search avec des paramètres spécifiques
            search_url = f"https://www.google.com/search?q={search_query}&num=3"
            
            print(f"🔍 Recherche Google pour: {city_name}")
            
            # Headers plus sophistiqués pour éviter la détection
            headers = {
                'User-Agent': 'Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36',
                'Accept': 'text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8',
                'Accept-Language': 'en-US,en;q=0.9',
                'Accept-Encoding': 'gzip, deflate, br',
                'Connection': 'keep-alive',
                'Sec-Fetch-Dest': 'document',
                'Sec-Fetch-Mode': 'navigate',
                'Sec-Fetch-Site': 'none',
                'Sec-Fetch-User': '?1',
                'Upgrade-Insecure-Requests': '1',
            }
            
            response = requests.get(search_url, headers=headers, timeout=15)
            response.raise_for_status()
            
            soup = BeautifulSoup(response.content, 'html.parser')
            
            # Extraire les liens des résultats Google
            links = []
            for result in soup.find_all('a', href=True):
                href = result['href']
                if href.startswith('/url?q='):
                    # Extraire l'URL réelle
                    actual_url = href.split('/url?q=')[1].split('&')[0]
                    actual_url = unquote(actual_url)
                    if actual_url.startswith('http') and 'google.com' not in actual_url:
                        links.append(actual_url)
                        if len(links) >= 3:
                            break
            
            # Extraire le contenu
            for link in links:
                content = self.extract_content(link)
                if content and len(content) > 200:
                    print(f"✅ Contenu extrait de: {link[:50]}...")
                    return content[:self.output_length]
            
            return ""
            
        except Exception as e:
            print(f"❌ Erreur recherche Google: {e}")
            return ""


    ########################################################################

    def search_web(self, city_name: str) -> str:
        """Méthode principale qui essaie différentes sources"""
        print(f"🔍 Recherche pour: {city_name}")
        
        # Essayer Wikipedia en premier (plus fiable)
        content = self.search_web_wikipedia(city_name)
        if content:
            return content
            
        # Puis les sources directes
        content = self.search_web_direct_sources(city_name)
        if content:
            return content
            
        # En dernier recours, Google (plus risqué)
        content = self.search_web_google(city_name)
        if content:
            return content
            
        print("❌ Aucun contenu trouvé")
        return ""

    ########################################################################

    def extract_content(self, url: str) -> str:
        """Extrait et nettoie le contenu HTML d'une page"""
        try:
            response = self.session.get(url, timeout=10)
            response.raise_for_status()
            return self.extract_content_from_response(response)
            
        except Exception as e:
            print(f"❌ Erreur extraction contenu de {url}: {e}")
            return ""
        
    def extract_content_from_response(self, response) -> str:
        """Extrait le contenu d'une réponse HTTP"""
        try:
            soup = BeautifulSoup(response.content, 'html.parser')
            
            # Supprimer les éléments indésirables
            for element in soup(['script', 'style', 'nav', 'header', 'footer', 'aside', 'advertisement']):
                element.decompose()
            
            # Sélecteurs spécifiques par site
            content_selectors = [
                # Wikipedia
                '.mw-parser-output p',
                '#mw-content-text p',
                # Britannica
                '.topic-paragraph',
                # Général
                'article', 'main', '.content', '#content', 
                '.post-content', '.entry-content', '.article-content'
            ]
            
            content = ""
            for selector in content_selectors:
                elements = soup.select(selector)
                if elements:
                    # Prendre les premiers paragraphes
                    texts = [elem.get_text(separator=' ', strip=True) for elem in elements[:3]]
                    content = ' '.join(texts)
                    break
            
            # Si pas de contenu spécifique, prendre le body
            if not content:
                body = soup.find('body')
                if body:
                    content = body.get_text(separator=' ', strip=True)
            
            # Nettoyer le texte
            content = re.sub(r'\s+', ' ', content)
            content = re.sub(r'\n+', '\n', content)
            
            return content.strip()
            
        except Exception as e:
            print(f"❌ Erreur extraction: {e}")
            return ""

    ########################################################################

if __name__ == "__main__":
    scraper = WebScraper()
    city_name = "Gyaur Kala"
    start_time = time.time()
    content = scraper.search_web(city_name)
    print(f"Temps de recherche: {time.time() - start_time:.2f} secondes")
    print(content[:10000])