from code.json_handler import JSONHandler
from code.AI_agent import AiAgent
from code.web_scraper import WebScraper

if __name__ == '__main__':
    ai_agent = AiAgent(ollama_url="http://localhost:11434", model="phi3:mini")
    web_scraper = WebScraper()
    
    json_handler = JSONHandler(ai_agent=ai_agent, web_scraper=web_scraper, print_bool=False)
    
    create_base_dict = False
    enrich_dict = False
    create_texture = True

    if create_base_dict:
        json_handler.create_dictionary()
        json_handler.create_intermediate_data(output_path="output/")
    if enrich_dict:
        json_handler.enrich_data_with_ai(input_file="output/intermediate_cities_result.json")
        json_handler.create_enriched_output_data(output_path="output/")
    if create_texture:
        json_handler.create_texture(output_path="output/", texture_width=2*8640, texture_height=2*4320)