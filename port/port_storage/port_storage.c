/**
 * @file      port_storage.c
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
#include "util_common_log.h"

//
#include <stdint.h>
#include <string.h>  // for memcpy

#define LOG_LEVEL LOG_LEVEL_DEBUG
#define TAG "PORT_PARTITION"

// --- Static helper function signatures ---
static int check_flash_address_alignment(uint32_t address);
static int erase_flash_sectors(uint32_t startAddress, uint32_t dataSizeBytes);
static int program_flash_word(uint32_t address, const uint8_t* data16);

// --- Port functions ---

int port_storage_init(void) { return 0; }

uint32_t port_storage_get_page_size(void) { return 0; }

int port_storage_write_bytes(const uint32_t startAddress, const uint8_t* pDataBuff,
                             uint32_t dataBuffSizeBytes) {
    if (pDataBuff == NULL || dataBuffSizeBytes == 0) {
        return -1;
    }

    return 0;
}

int port_storage_read_bytes(const uint32_t startAddress, uint8_t* pDataBuff,
                            uint32_t dataBuffSizeBytes) {
    if (pDataBuff == NULL || dataBuffSizeBytes == 0) {
        return -1;
    }

#if LOG_LEVEL >= LOG_LEVEL_DEBUG
    SERVICE_LOGD(TAG, "Reading %lu bytes from address 0x%08lX", dataBuffSizeBytes, startAddress);
#endif

    return -1;
}

// --- Static helper functions ---

static int check_flash_address_alignment(uint32_t address) { return 0; }

static int erase_flash_sectors(uint32_t startAddress, uint32_t dataSizeBytes) { return 0; }

static int program_flash_word(uint32_t address, const uint8_t* data16) { return 0; }
