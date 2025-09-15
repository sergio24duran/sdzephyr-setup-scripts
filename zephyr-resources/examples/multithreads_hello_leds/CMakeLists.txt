cmake_minimum_required(VERSION 3.13.1)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(multithreads_hello_leds)

target_sources(app PRIVATE
    main/src/main.c
    led14/src/led14_main.c
    led18/src/led18_main.c
)
