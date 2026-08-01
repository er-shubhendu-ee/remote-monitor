# mongoose/CMakeLists.txt

# Register the 'mongoose' component with ESP-IDF
idf_component_register(
    SRCS "mongoose.c"       # Source file(s) for this component
    INCLUDE_DIRS "."           # Directories containing headers for this component; '.' is the same folder as mongoose.c

    PRIV_REQUIRES
            esp_system      #
            esp_timer       #
            app_update      #
            fatfs
)

# Explanation:
# - SRCS specifies which source files to compile for the mongoose component.
# - INCLUDE_DIRS "." ensures that mongoose.c can include headers from its own folder (like mongoose.h).
# - PRIV_REQUIRES spi_flash tells ESP-IDF that mongoose depends on spi_flash:
#     * spi_flash headers will be available during compilation of mongoose.c
#     * spi_flash component will be built before mongoose
# - Any other component (like main or app) that wants to include mongoose.h must explicitly
#   declare a dependency on 'mongoose' in its REQUIRES or PRIV_REQUIRES list; otherwise,
#   the compiler will not know the header path and will fail with "No such file or directory".
