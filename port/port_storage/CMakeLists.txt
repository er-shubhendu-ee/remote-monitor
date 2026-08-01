# port_storage/CMakeLists.txt

# Register the 'port_storage' component with ESP-IDF
idf_component_register(
    SRCS "port_storage.c"       # Source file(s) for this component
    INCLUDE_DIRS "."           # Directories containing headers for this component; '.' is the same folder as port_storage.c

    PRIV_REQUIRES   spi_flash    # Declare a private dependency on the 'spi_flash' component
                    util_common
)

# Explanation:
# - SRCS specifies which source files to compile for the port_storage component.
# - INCLUDE_DIRS "." ensures that port_storage.c can include headers from its own folder (like port_storage.h).
# - PRIV_REQUIRES spi_flash tells ESP-IDF that port_storage depends on spi_flash:
#     * spi_flash headers will be available during compilation of port_storage.c
#     * spi_flash component will be built before port_storage
# - Any other component (like main or app) that wants to include port_storage.h must explicitly
#   declare a dependency on 'port_storage' in its REQUIRES or PRIV_REQUIRES list; otherwise,
#   the compiler will not know the header path and will fail with "No such file or directory".
