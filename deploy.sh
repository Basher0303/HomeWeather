#!/bin/bash

source ./deploy.config.sh

# Цвета
GREEN="\033[1;32m"
BLUE="\033[1;34m"
YELLOW="\033[1;33m"
RED="\033[1;31m"
RESET="\033[0m"


# Парсинг аргументов
SERVICES=()

for arg in "$@"; do
    case $arg in
        back|backend)
            SERVICES+=("backend")
            ;;
        all)
            SERVICES=("frontend" "backend")
            ;;
        *)
            echo -e "${YELLOW}Неизвестный аргумент: $arg${RESET}"
            ;;
    esac
done

function should_deploy() {
    local service=$1
    [[ " ${SERVICES[*]} " =~ " ${service} " ]]
}

function section() {
    echo -e "\n${BLUE}=== $1 ===${RESET}"
}

function success() {
    echo -e "${GREEN}✔ $1${RESET}"
}

function warn() {
    echo -e "${YELLOW}⚠ $1${RESET}"
}

function error() {
    echo -e "${RED}✖ $1${RESET}"
    exit 1
}


# Проверка аргументов
if [ ${#SERVICES[@]} -eq 0 ]; then
    echo -e "${YELLOW}Использование: ./deploy.sh [front|back|docker]...${RESET}"
    echo -e "${YELLOW}Примеры:${RESET}"
    echo -e "  ${GREEN}./deploy.sh back${RESET} - бэкенд"  
    exit 1
fi

section "Старт деплоя"
if [ ${#SERVICES[@]} -gt 0 ]; then
    echo -e "${YELLOW}Сервисы:${RESET} ${GREEN}${SERVICES[*]}${RESET}"
fi

if should_deploy "backend"; then
    section "Копирование бэкенда"

    rsync -avzh --delete \
        -e "ssh -i $SSH_KEY -p $SSH_PORT" \
        --rsync-path="mkdir -p $PROJECT_DIR/backend && rsync" \
        ./backend/ \
        "$SSH_USER@$SERVER_IP:$PROJECT_DIR/backend/" \
        --exclude='node_modules' \
        --exclude='src/db/db.json' && success "Бэкенд скопирован"

    ssh -i $SSH_KEY -p $SSH_PORT $SSH_USER@$SERVER_IP "
        export NVM_DIR=\$HOME/.nvm && \
        . \$NVM_DIR/nvm.sh && \
        cd $PROJECT_DIR/backend && \
        node -v && npm -v && \
        npm ci \ 
        if pm2 describe backend-weather > /dev/null 2>&1; then
            pm2 reload backend-weather
        else
            pm2 start ecosystem.config.cjs
        fi
        pm2 save
    " && success "Бэкенд запущен"
fi
section "Деплой завершён"