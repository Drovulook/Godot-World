#!/bin/bash
# filepath: /home/PL/Documents/projets_Godot/World/run_project.sh

# Couleurs pour les messages
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}🚀 Démarrage de la génération des données villes${NC}"

# 1. Démarrer Ollama en arrière-plan avec logs
echo -e "${YELLOW}📝 Démarrage d'Ollama...${NC}"
if pgrep -x "ollama" > /dev/null; then
    echo -e "${GREEN}✅ Ollama est déjà en cours d'exécution${NC}"
else
    ollama serve > ollama_logs.txt 2>&1 &
    OLLAMA_PID=$!
    echo -e "${GREEN}✅ Ollama démarré (PID: $OLLAMA_PID)${NC}"
    echo -e "${YELLOW}⏳ Attente de 3 secondes pour le démarrage complet...${NC}"
    sleep 3
fi

# 2. Vérifier si un environnement virtuel est activé
if [[ -z "$VIRTUAL_ENV" ]]; then
    echo -e "${YELLOW}🐍 Activation de l'environnement Python...${NC}"
    
    # Chercher un environnement virtuel dans les endroits courants
    if [[ -d "env" ]]; then
        source env/bin/activate
        echo -e "${GREEN}✅ Environnement 'env' activé${NC}"
    elif [[ -d ".env" ]]; then
        source .env/bin/activate
        echo -e "${GREEN}✅ Environnement '.env' activé${NC}"
    elif [[ -d "env" ]]; then
        source env/bin/activate
        echo -e "${GREEN}✅ Environnement 'env' activé${NC}"
    else
        echo -e "${RED}⚠️ Aucun environnement virtuel trouvé dans le répertoire courant${NC}"
        echo -e "${YELLOW}Création d'un nouvel environnement virtuel...${NC}"
        python3 -m env env
        source env/bin/activate
        echo -e "${GREEN}✅ Nouvel environnement 'env' créé et activé${NC}"
    fi
else
    echo -e "${GREEN}✅ Environnement Python déjà activé: $VIRTUAL_ENV${NC}"
fi

# 3. Vérifier que main.py existe
if [[ ! -f "main.py" ]]; then
    echo -e "${RED}❌ Erreur: main.py introuvable dans le répertoire courant${NC}"
    exit 1
fi

# 4. Exécuter main.py
echo -e "${YELLOW}🎯 Exécution de main.py...${NC}"
python main.py

# 5. Capture du code de sortie
EXIT_CODE=$?

if [[ $EXIT_CODE -eq 0 ]]; then
    echo -e "${GREEN}✅ main.py s'est exécuté avec succès${NC}"
else
    echo -e "${RED}❌ main.py s'est terminé avec le code d'erreur: $EXIT_CODE${NC}"
fi

echo -e "${GREEN}🏁 Fin du script${NC}"
exit $EXIT_CODE