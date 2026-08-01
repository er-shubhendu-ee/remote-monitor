# port_system/CMakeLists.txt

# Register the 'port_system' component with ESP-IDF
idf_component_register(
    SRCS "port_system.c"       # Source file(s) for this component
    INCLUDE_DIRS "."           # Directories containing headers for this component; '.' is the same folder as port_system.c

    PRIV_REQUIRES spi_flash    # Declare a private dependency on the 'spi_flash' component
)

# Explanation:
# - SRCS specifies which source files to compile for the port_system component.
# - INCLUDE_DIRS "." ensures that port_system.c can include headers from its own folder (like port_system.h).
# - PRIV_REQUIRES spi_flash tells ESP-IDF that port_system depends on spi_flash:
#     * spi_flash headers will be available during compilation of port_system.c
#     * spi_flash component will be built before port_system
# - Any other component (like main or app) that wants to include port_system.h must explicitly
#   declare a dependency on 'port_system' in its REQUIRES or PRIV_REQUIRES list; otherwise,
#   the compiler will not know the header path and will fail with "No such file or directory".
