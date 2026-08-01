/**
 * @file      util_common_log.h
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
#include <stdio.h>

// Define log levels
#define LOG_LEVEL_NONE 0
#define LOG_LEVEL_ERROR 1
#define LOG_LEVEL_WARN 2
#define LOG_LEVEL_INFO 3
#define LOG_LEVEL_DEBUG 4
#define LOG_LEVEL_VERBOSE 5

// Color codes
#define COLOR_RESET "\033[0m"
#define COLOR_DEBUG "\033[37m"
#define COLOR_INFO "\033[32m"
#define COLOR_WARN "\033[33m"
#define COLOR_ERROR "\033[31m"

// Logging macros with color formatting for terminal output

#define SERVICE_LOGD(TAG, fmt, ...) \
    printf(COLOR_DEBUG TAG " : %d: " fmt COLOR_RESET "\r\n", __LINE__, ##__VA_ARGS__);

#define SERVICE_LOGI(TAG, fmt, ...) \
    printf(COLOR_INFO TAG " : %d: " fmt COLOR_RESET "\r\n", __LINE__, ##__VA_ARGS__);

#define SERVICE_LOGW(TAG, fmt, ...) \
    printf(COLOR_WARN TAG " : %d: " fmt COLOR_RESET "\r\n", __LINE__, ##__VA_ARGS__);

#define SERVICE_LOGE(TAG, fmt, ...) \
    printf(COLOR_ERROR TAG " : %d: " fmt COLOR_RESET "\r\n", __LINE__, ##__VA_ARGS__);
