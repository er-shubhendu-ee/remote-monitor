# main/CMakeLists.txt

# Register the 'main' component with ESP-IDF
idf_component_register(
    SRCS "main.c"             # Source file(s) for this component

    PRIV_REQUIRES             # Private dependencies: these components must be built before 'main' 
        spi_flash             # ESP-IDF built-in SPI flash component
        app                   # Our 'app' component; its headers will be added to main's include path
        port_system           # 'port_system' component; ensures its headers (port_system.h) are visible to main

    INCLUDE_DIRS ""           # Additional local include directories for this component; empty here
)

# Explanation:
# - SRCS lists the source files compiled for the 'main' component.
# - PRIV_REQUIRES is critical: it tells CMake/ESP-IDF which components this one depends on.
#     * Only components listed here (or in REQUIRES) will have their headers automatically included
#       in the compiler include paths when building main.c.
#     * Without port_system in this list, #include "port_system.h" in main.c will fail, 
#       producing a "No such file or directory" error.
# - INCLUDE_DIRS "" means no extra local include paths are added; the compiler relies on the include paths
#   from PRIV_REQUIRES components and the main component's own folder.
# - The order in PRIV_REQUIRES can matter if multiple components provide headers with the same name.
