# Create the library target
add_library(led
    STATIC
    src/led.c
)

# Add the library's header file(s)
target_include_directories(
    led
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Required to set Zephyr compile options (e.g. for ARM)
target_link_libraries(
    led
    PRIVATE
    zephyr_interface
)
