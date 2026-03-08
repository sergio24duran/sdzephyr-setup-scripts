# ---------------------------------------------------------------------------
# CMakeLists.txt - Build System Configuration
# ---------------------------------------------------------------------------
# Zephyr uses CMake as its build system. This file tells CMake:
#   1. The minimum CMake version required
#   2. To find and load the Zephyr build system
#   3. The project name
#   4. Which source files to compile
#
# The Zephyr CMake package automatically handles:
#   - Cross-compilation toolchain setup
#   - Linking against the Zephyr kernel and enabled libraries
#   - Processing Kconfig options and devicetree overlays
# ---------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.20.0)

# Find the Zephyr package. ZEPHYR_BASE env variable must point to the
# Zephyr repository (set automatically by "west build" or "source zephyr-env.sh")
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(hello_led)

# Add application source files to the "app" target (provided by Zephyr)
target_sources(app PRIVATE src/main.c)
