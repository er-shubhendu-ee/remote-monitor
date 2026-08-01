# util_string/CMakeLists.txt

# Register the util_string component with ESP-IDF
idf_component_register(
    SRCS "util_string.c"      # Source file(s) for this component
    INCLUDE_DIRS "."         # Directories containing headers to be added to this component's include path
)


