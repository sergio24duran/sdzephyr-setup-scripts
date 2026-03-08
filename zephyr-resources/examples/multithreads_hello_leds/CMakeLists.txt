# ---------------------------------------------------------------------------
# CMakeLists.txt - Multithreaded LED Example
# ---------------------------------------------------------------------------
# This project has multiple source files, one per thread. Each file
# implements a thread entry function. CMake compiles them all and links
# them into a single firmware image.
# ---------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(multithreads_hello_leds)

# All source files are compiled together into one binary.
# In Zephyr, threads share the same address space (no process isolation),
# so functions defined in one file can be called from another.
target_sources(app PRIVATE
    src/main.c
    src/led_a_thread.c
    src/led_b_thread.c
)
