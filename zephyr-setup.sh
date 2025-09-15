#!/usr/bin/env bash

# ================================
# CONFIGURATION
# ================================
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

# Lista de dependencias -> comando para instalar
declare -A deps=(
    ["west"]="sudo apt install -y python3-pip && pip3 install west"
    ["cmake"]="sudo apt install -y cmake"
    ["ninja"]="sudo apt install -y ninja-build"
    ["dtc"]="sudo apt install -y device-tree-compiler"
    ["python3"]="sudo apt install -y python3 python3-pip"
    ["esptool"]="pip3 install esptool"
)

echo "🔍 Verificando dependencias necesarias para Zephyr..."

missing=0

for dep in "${!deps[@]}"; do
    if ! command -v "$dep" &> /dev/null; then
        echo "❌ Falta: $dep"
        echo "   👉 Instálalo con:"
        echo "      ${deps[$dep]}"
        missing=1
    else
        echo "✅ $dep encontrado en: $(command -v $dep)"
    fi
done

if [ $missing -eq 0 ]; then
    echo "🎉 Todas las dependencias están instaladas."
else
    echo -e "\n⚠️ Instala las dependencias faltantes y vuelve a ejecutar este script."
    exit 1
fi

########################################
### Paso 2: Leer argumentos
########################################
PROJECT_PATH=""

while [[ $# -gt 0 ]]; do
    case "$1" in
        -p)
            PROJECT_PATH="$2"
            shift 2
            ;;
        *)
            echo "❌ Argumento desconocido: $1"
            echo "Uso: $0 -p /ruta/absoluta/proyecto"
            exit 1
            ;;
    esac
done

if [ -z "$PROJECT_PATH" ]; then
    echo "❌ Debes indicar la ruta del proyecto con -p"
    echo "Uso: $0 -p /ruta/absoluta/proyecto"
    exit 1
fi

# Validar que el path sea absoluto
if [[ "$PROJECT_PATH" != /* ]]; then
    echo "❌ El path debe ser absoluto (debe empezar con '/')."
    echo "👉 Ejemplo: $0 -p /home/user/miproyecto"
    exit 1
fi

echo "📂 Ruta del proyecto: $PROJECT_PATH"

########################################
### Paso 3: Iniciar Zephyr en el path
########################################
mkdir -p "$PROJECT_PATH"
cd "$PROJECT_PATH"

echo "🚀 Inicializando proyecto Zephyr con 'west init' en "$PROJECT_PATH"/ ..."
west init 

cd "$PROJECT_PATH"/zephyr

echo "🔄 Actualizando Zephyr con 'west update'..."
west update

echo "📦 Exportando Zephyr para la construcción..."
west zephyr-export

echo "⚙️ Instalando dependencias de Python de Zephyr..."
pipx runpip west install -r scripts/requirements.txt

echo "✅ Proyecto Zephyr listo en: "$PROJECT_PATH"/zephyr"

# ========================================
# Step 4: Copy resources
# ========================================
RESOURCE_DIR="${SCRIPT_DIR}/zephyr-resources"

# Copy gitignore
GITIG_SRC="${RESOURCE_DIR}/.gitignore"
if [[ ! -f "${GITIG_SRC}" ]]; then
  echo "❌ File not found: ${GITIG_SRC}" >&2
  exit 1
fi
cp "${GITIG_SRC}" "${PROJECT_PATH}/" || { echo "❌ Error when trying to copy .gitignore" >&2; exit 1; }

# Copy examples directory
EXAMPLES_SRC="${RESOURCE_DIR}/examples"
if [[ ! -d "${EXAMPLES_SRC}" ]]; then
  echo "❌ examples directory not found: ${EXAMPLES_SRC}" >&2
  exit 1
fi
cp -r "${EXAMPLES_SRC}" "${PROJECT_PATH}/" || { echo "❌ Error when trying to copy examples directory" >&2; exit 1; }

echo "🎯 Zephyr project is ready at: $PROJECT_PATH"