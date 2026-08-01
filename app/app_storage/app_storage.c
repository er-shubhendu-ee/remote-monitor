/**
 * @file      app_storage.c
 * @author:   Shubhendu B B
 * @date:     02/08/2026
 * @brief
 * @details   Distributed globally for free under the MIT License terms.
 *
 * @copyright Copyright (c) 2025 er-shubhendu-ee
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 **/

#include "app_storage.h"

//
#include "util_common_error.h"
#include "util_common_log.h"

//
#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "wear_levelling.h"

//
#include <dirent.h>    // for opendir, readdir, closedir
#include <sys/stat.h>  // for stat()

//
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOG_LEVEL LOG_LEVEL_DEBUG
#define TAG "APP"

// Handle for wear levelling driver
static wl_handle_t s_wl_handle = WL_INVALID_HANDLE;

/**
 * @brief Initialize SPI flash FATFS
 */
int app_storage_Init(const char* pBasePathSrtSz) {
    ESP_LOGI(TAG, "Mounting FAT filesystem at %s", pBasePathSrtSz);

    const esp_vfs_fat_mount_config_t mount_config = {
        .format_if_mount_failed = false,  // Format if mount fails
        .max_files = 5,                   // Max simultaneously open files
        .allocation_unit_size = 4096,     // Typically 4096
        .use_one_fat = false,
    };

    esp_err_t err =
        esp_vfs_fat_spiflash_mount(pBasePathSrtSz, "fatfs", &mount_config, &s_wl_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount FATFS (%s)", esp_err_to_name(err));
        return -1;
    }

    printf("Storage mounted at %s\n", pBasePathSrtSz);

    // ---- List top-level content ----
    printf("Contents of %s:\n", pBasePathSrtSz);
    DIR* dir = opendir(pBasePathSrtSz);
    if (!dir) {
        printf("Failed to open directory: %s\n", pBasePathSrtSz);
        return -1;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        printf(" - %s\n", entry->d_name);
    }

    closedir(dir);

    ESP_LOGI(TAG, "Storage initialized successfully");
    return NO_ERROR;
}

/**
 * @brief Deinitialize SPI flash FATFS
 */
int app_storage_Deinit(char* pBasePathSrtSz) {
    ESP_LOGI(TAG, "Unmounting FAT filesystem at %s", pBasePathSrtSz);

    esp_err_t err = esp_vfs_fat_spiflash_unmount(pBasePathSrtSz, s_wl_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to unmount FATFS (%s)", esp_err_to_name(err));
        return -1;
    }

    s_wl_handle = WL_INVALID_HANDLE;
    ESP_LOGI(TAG, "Storage deinitialized successfully");
    return NO_ERROR;
}
