/**
 * @file      port_storage.h
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

#ifndef _PORT_STORAGE_H_
#define _PORT_STORAGE_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int port_storage_init(void);

uint32_t port_storage_get_page_size(void);

/**
 * @brief Write raw bytes to the system partition starting at a given address.
 *
 * @param[in] startAddress Start address to write to.
 * @param[in] pDataBuff Pointer to data buffer to write.
 * @param[in] dataBuffSizeBytes Number of bytes to write.
 *
 * @return 0 on success, negative error code otherwise.
 */
int port_storage_write_bytes(const uint32_t startAddress, const uint8_t* pDataBuff,
                             uint32_t dataBuffSizeBytes);

/**
 * @brief Read raw bytes from the system partition starting at a given address.
 *
 * @param[in] startAddress Start address to read from.
 * @param[out] pDataBuff Pointer to buffer to store read data.
 * @param[in] dataBuffSizeBytes Number of bytes to read.
 *
 * @return 0 on success, negative error code otherwise.
 */
int port_storage_read_bytes(const uint32_t startAddress, uint8_t* pDataBuff,
                            uint32_t dataBuffSizeBytes);

#ifdef __cplusplus
}
#endif

#endif /* @end  _PORT_STORAGE_H_*/