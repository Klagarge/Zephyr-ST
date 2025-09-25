# Minimum CMake version
cmake_minimum_required(VERSION 3.20.0)

# Locate the Zephyr RTOS source
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

# Name the project
project(demo)

target_include_directories(app PRIVATE ${ZEPHYR_BASE}/include/zephyr/portability)

add_subdirectory(lib/led)

# Locate the source code for the application
file(GLOB_RECURSE APP_SOURCES 
    "src/*.c"
    "src/*.s"
)
target_sources(app PRIVATE ${APP_SOURCES})

# Link the library to the application
target_link_libraries(app PRIVATE
    led
)
