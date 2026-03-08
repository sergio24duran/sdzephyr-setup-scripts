#!/usr/bin/env bash
set -euo pipefail

# ==========================================================================
# zephyr-setup.sh - Automated Zephyr RTOS Project Setup
# ==========================================================================
# This script automates the setup of a new Zephyr RTOS project:
#   1. (Optional) Checks and installs required host dependencies
#   2. Initializes a Zephyr workspace using West (Zephyr's meta-tool)
#   3. Installs Zephyr's Python dependencies
#   4. (Optional) Copies example projects and .gitignore into the workspace
#
# Usage:
#   ./zephyr-setup.sh -p /absolute/path/to/project [-d] [-y] [-e] [-h]
#
# Options:
#   -p PATH   Absolute path where the Zephyr project will be created
#   -d        Skip dependency check entirely (manage dependencies yourself)
#   -y        Auto-install missing dependencies (invalid when -d is set)
#   -e        Copy bundled example projects into the workspace
#   -h        Show this help message
# ==========================================================================

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

# ==========================================================================
# Color helpers (disabled if stdout is not a terminal)
# ==========================================================================
if [ -t 1 ]; then
    RED='\033[0;31m'
    GREEN='\033[0;32m'
    YELLOW='\033[0;33m'
    CYAN='\033[0;36m'
    NC='\033[0m' # No Color
else
    RED='' GREEN='' YELLOW='' CYAN='' NC=''
fi

info()  { echo -e "${CYAN}[INFO]${NC}  $*"; }
ok()    { echo -e "${GREEN}[OK]${NC}    $*"; }
warn()  { echo -e "${YELLOW}[WARN]${NC}  $*"; }
error() { echo -e "${RED}[ERROR]${NC} $*" >&2; }

# ==========================================================================
# Help message
# ==========================================================================
usage() {
    echo "Usage: $0 -p /absolute/path/to/project [-d] [-y] [-e] [-h]"
    echo ""
    echo "Options:"
    echo "  -p PATH   Absolute path where the Zephyr project will be created (required)"
    echo "  -d        Skip dependency check entirely (manage dependencies yourself)"
    echo "  -y        Auto-install missing dependencies without prompting"
    echo "            (invalid when combined with -d)"
    echo "  -e        Copy bundled example projects into the workspace"
    echo "  -h        Show this help message"
    echo ""
    echo "Examples:"
    echo "  # Minimal setup, manage dependencies yourself, no examples"
    echo "  $0 -p /home/user/my-zephyr-project -d"
    echo ""
    echo "  # Full setup with auto-install and examples"
    echo "  $0 -p /home/user/my-zephyr-project -y -e"
    echo ""
    echo "  # Check/install dependencies interactively, no examples"
    echo "  $0 -p /home/user/my-zephyr-project"
    exit 0
}

# ==========================================================================
# Step 1: Parse arguments
# ==========================================================================
PROJECT_PATH=""
SKIP_DEPS=false
AUTO_INSTALL=false
COPY_EXAMPLES=false

while [[ $# -gt 0 ]]; do
    case "$1" in
        -p)
            PROJECT_PATH="$2"
            shift 2
            ;;
        -d)
            SKIP_DEPS=true
            shift
            ;;
        -y)
            AUTO_INSTALL=true
            shift
            ;;
        -e)
            COPY_EXAMPLES=true
            shift
            ;;
        -h|--help)
            usage
            ;;
        *)
            error "Unknown argument: $1"
            usage
            ;;
    esac
done

# Validate required argument
if [ -z "$PROJECT_PATH" ]; then
    error "You must specify a project path with -p"
    usage
fi

if [[ "$PROJECT_PATH" != /* ]]; then
    error "The path must be absolute (must start with '/')."
    echo "  Example: $0 -p /home/user/my-zephyr-project"
    exit 1
fi

# -y is meaningless when -d is set
if [ "$SKIP_DEPS" = true ] && [ "$AUTO_INSTALL" = true ]; then
    error "Flags -d and -y are mutually exclusive: -d skips dependency checks entirely, so -y has no effect."
    echo "  Use -d alone to skip all dependency handling."
    exit 1
fi

info "Project path:   $PROJECT_PATH"
info "Skip dep check: $SKIP_DEPS"
info "Auto-install:   $AUTO_INSTALL"
info "Copy examples:  $COPY_EXAMPLES"
echo ""

# ==========================================================================
# Step 2: Check host dependencies (skipped with -d)
# ==========================================================================
if [ "$SKIP_DEPS" = true ]; then
    warn "Skipping dependency check (-d flag set). Make sure all required tools are installed."
    warn "Required tools: cmake, ninja, dtc, python3, pip3, gperf, ccache, gcc, west"
else
    # These are the tools required on the host system to build Zephyr projects.
    # See: https://docs.zephyrproject.org/latest/develop/getting_started/index.html

    # APT packages needed (package-name -> binary to check)
    declare -A apt_deps=(
        ["cmake"]="cmake"
        ["ninja-build"]="ninja"
        ["device-tree-compiler"]="dtc"
        ["python3"]="python3"
        ["python3-pip"]="pip3"
        ["python3-venv"]="python3"
        ["gperf"]="gperf"
        ["ccache"]="ccache"
        ["dfu-util"]="dfu-util"
        ["wget"]="wget"
        ["xz-utils"]="xz"
        ["file"]="file"
        ["make"]="make"
        ["gcc"]="gcc"
        ["g++"]="g++"
        ["libsdl2-dev"]="sdl2-config"
        ["git"]="git"
    )

    # Pip packages (checked separately)
    declare -A pip_deps=(
        ["west"]="west"
        ["esptool"]="esptool.py"
    )

    info "Checking host dependencies required for Zephyr..."
    echo ""

    missing_apt=()
    missing_pip=()

    for pkg in "${!apt_deps[@]}"; do
        bin="${apt_deps[$pkg]}"
        if command -v "$bin" &> /dev/null; then
            ok "$bin (package: $pkg)"
        else
            warn "Missing: $bin (package: $pkg)"
            missing_apt+=("$pkg")
        fi
    done

    for pkg in "${!pip_deps[@]}"; do
        bin="${pip_deps[$pkg]}"
        if command -v "$bin" &> /dev/null; then
            ok "$bin (pip: $pkg)"
        else
            warn "Missing: $bin (pip: $pkg)"
            missing_pip+=("$pkg")
        fi
    done

    echo ""

    # Attempt to install missing dependencies
    if [ ${#missing_apt[@]} -gt 0 ] || [ ${#missing_pip[@]} -gt 0 ]; then
        warn "Some dependencies are missing."

        if [ "$AUTO_INSTALL" = false ]; then
            echo ""
            echo "Missing APT packages: ${missing_apt[*]:-none}"
            echo "Missing pip packages:  ${missing_pip[*]:-none}"
            echo ""
            read -rp "Do you want to install them now? [y/N] " answer
            if [[ ! "$answer" =~ ^[Yy]$ ]]; then
                error "Please install the missing dependencies and re-run this script."
                exit 1
            fi
        fi

        if [ ${#missing_apt[@]} -gt 0 ]; then
            info "Installing APT packages: ${missing_apt[*]}"
            sudo apt update
            sudo apt install -y "${missing_apt[@]}"
        fi

        if [ ${#missing_pip[@]} -gt 0 ]; then
            info "Installing pip packages: ${missing_pip[*]}"
            pip3 install --user "${missing_pip[@]}"
        fi

        ok "All dependencies installed."
    else
        ok "All dependencies are already installed."
    fi
fi

echo ""

# ==========================================================================
# Step 3: Initialize Zephyr workspace with West
# ==========================================================================
info "Creating project directory: $PROJECT_PATH"
mkdir -p "$PROJECT_PATH"

if [ -d "$PROJECT_PATH/.west" ]; then
    warn "West workspace already exists at $PROJECT_PATH - skipping 'west init'."
else
    info "Initializing Zephyr workspace with 'west init'..."
    cd "$PROJECT_PATH"
    west init || { error "Failed to run 'west init'. Check your network connection."; exit 1; }
fi

info "Updating Zephyr modules with 'west update' (this may take a while)..."
cd "$PROJECT_PATH"
west update || { error "'west update' failed."; exit 1; }

info "Exporting Zephyr CMake package..."
west zephyr-export

info "Installing Zephyr Python dependencies..."
pip3 install --user -r "$PROJECT_PATH/zephyr/scripts/requirements.txt"

ok "Zephyr workspace initialized at: $PROJECT_PATH"
echo ""

# ==========================================================================
# Step 4: Copy resources (examples + .gitignore) - only with -e
# ==========================================================================
RESOURCE_DIR="${SCRIPT_DIR}/zephyr-resources"

# Always copy .gitignore
GITIG_SRC="${RESOURCE_DIR}/.gitignore"
if [[ ! -f "${GITIG_SRC}" ]]; then
    error "File not found: ${GITIG_SRC}"
    exit 1
fi
cp "${GITIG_SRC}" "${PROJECT_PATH}/" || { error "Failed to copy .gitignore"; exit 1; }
ok "Copied .gitignore"

# Copy examples only if -e was passed
if [ "$COPY_EXAMPLES" = true ]; then
    EXAMPLES_SRC="${RESOURCE_DIR}/examples"
    if [[ ! -d "${EXAMPLES_SRC}" ]]; then
        error "Examples directory not found: ${EXAMPLES_SRC}"
        exit 1
    fi
    cp -r "${EXAMPLES_SRC}" "${PROJECT_PATH}/" || { error "Failed to copy examples directory"; exit 1; }
    ok "Copied example projects to: $PROJECT_PATH/examples/"
else
    info "Skipping examples (pass -e to include them)."
fi

# ==========================================================================
# Done!
# ==========================================================================
echo ""
echo "=========================================="
info "Zephyr project is ready!"
echo "=========================================="
echo ""
echo "  Project location:  $PROJECT_PATH"
echo "  Zephyr source:     $PROJECT_PATH/zephyr"
if [ "$COPY_EXAMPLES" = true ]; then
echo "  Example projects:  $PROJECT_PATH/examples/"
fi
echo ""
echo "  Next steps:"
echo "    1. cd $PROJECT_PATH"
echo "    2. source zephyr/zephyr-env.sh"
if [ "$COPY_EXAMPLES" = true ]; then
echo "    3. west build -b <your_board> examples/hello_led"
echo "    4. west flash"
else
echo "    3. west build -b <your_board> <path/to/your/app>"
echo "    4. west flash"
fi
echo ""
echo "  To see available boards:"
echo "    west boards                       # list all 500+ supported boards"
echo "    west boards | grep esp32          # filter for ESP32 boards"
echo "    west boards | grep nrf            # filter for Nordic boards"
echo "    west boards | grep nucleo         # filter for STM32 Nucleo boards"
echo ""
echo "  To explore Kconfig options:"
echo "    west build -t menuconfig"
echo ""
