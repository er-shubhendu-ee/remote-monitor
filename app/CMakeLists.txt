# app/CMakeLists.txt

# Register the 'app' component with ESP-IDF
idf_component_register(
    SRCS    
            "app.c"
            "./app_client/app_client.c"
            "./app_gpio/app_gpio.c"
            "./app_helper/app_helper.c"
            "./app_server/app_server.c"
            "./app_sntp/app_sntp.c"
            "./app_storage/app_storage.c"
            "./app_wifi/app_wifi.c"

    INCLUDE_DIRS
            "."
            "./app_client"
            "./app_gpio"
            "./app_helper"
            "./app_server"
            "./app_sntp"
            "./app_storage"
            "./app_wifi"
            "./app_measurement"
            "./config"
      
    PRIV_REQUIRES
            esp_system      #
            esp_timer       #
            esp_wifi        # For Wi-Fi APIs
            esp_event       # For event loop
            nvs_flash       # For NVS initialization
            vfs             # Virtual File System (VFS) layer
            fatfs           # FAT filesystem implementation
            wear_levelling  # FATFS on SPI flash needs wear leveling
            mdns
            
            app_update
            
            util_common     # Common project utilities
            mongoose
)


# Explanation:
# - SRCS specifies the source files that will be compiled for this component.
# - INCLUDE_DIRS "." ensures that app.c can include headers from its own folder (e.g., app.h).
# - PRIV_REQUIRES app_helper tells ESP-IDF that this component depends on app_helper:
#     * During compilation, app_helper's include paths will be added to app's compiler include paths.
#     * app_helper will be built before app.
# - This dependency is crucial: any headers from app_helper (like app_helper.h) used in app.c
#   require app_helper to be listed here; otherwise, the compiler will not find them.
# - Components not listed in REQUIRES or PRIV_REQUIRES will not have their include paths automatically added.
