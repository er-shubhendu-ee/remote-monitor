# Root CMakeLists.txt

# Required CMake version for ESP-IDF projects
cmake_minimum_required(VERSION 3.16)

# Include ESP-IDF project helper functions and macros
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Enable minimal build: only build main and its required components
idf_build_set_property(MINIMAL_BUILD ON)

# Specify additional component directories outside the default 'components' and 'main'
# Each folder listed here must contain a CMakeLists.txt with idf_component_register()
# This makes the components discoverable by ESP-IDF/CMake
# Note: Being listed here does NOT automatically expose their headers to other components.
set(EXTRA_COMPONENT_DIRS
    "${CMAKE_SOURCE_DIR}/mongoose"                  # mongoose component
    
    "${CMAKE_SOURCE_DIR}/util/util_common"          # service_base component

    "${CMAKE_SOURCE_DIR}/port/port_storage"         # service_base component
    "${CMAKE_SOURCE_DIR}/port/port_system"          # service_base component

    "${CMAKE_SOURCE_DIR}/service/service_base"      # service_base component

    "${CMAKE_SOURCE_DIR}/app/app_helper"            # app_helper component
    "${CMAKE_SOURCE_DIR}/app"                       # app component
)

# Define the project name
project(gomukh-controller)

# Explanation:
# - EXTRA_COMPONENT_DIRS ensures that ESP-IDF can find these component folders when resolving dependencies.
# - Headers from these components are only added to a component's include path if that component
#   declares them in REQUIRES or PRIV_REQUIRES.
# - For example, main.c will only see port_system.h if main declares PRIV_REQUIRES port_system.
