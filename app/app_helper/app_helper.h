/**
 * @file      app_helper.h
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

#ifndef _APP_HELPER_H_
#define _APP_HELPER_H_

#include "util_common_error.h"
#include "util_common_log.h"

//
#include "app_config.h"
#include "app_helper.h"
#include "app_server.h"

//
#include "mongoose.h"

//
#include "FreeRTOSConfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

//
#include <arpa/inet.h>  // for inet_ntop
#include <ctype.h>
#include <netinet/in.h>  // for inet_ntop
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define APP_HELPER_ASSERT(statusCode, okFmt, errFmt, ...) \
    do {                                                  \
        if ((statusCode) == NO_ERROR) {                   \
            SERVICE_LOGI(TAG, okFmt, ##__VA_ARGS__);      \
        } else {                                          \
            SERVICE_LOGE(TAG, errFmt, ##__VA_ARGS__);     \
        }                                                 \
    } while (0)

// Macro: logs free stack every interval_ms milliseconds
#define APP_HELPER_LOG_FREE_STACK_AT_INTERVAL(TAG, interval_ms)               \
    do {                                                                      \
        static TickType_t _lastLogTick = 0;                                   \
        TickType_t _now = xTaskGetTickCount();                                \
        if ((_now - _lastLogTick) >= pdMS_TO_TICKS(interval_ms)) {            \
            _lastLogTick = _now;                                              \
            SERVICE_LOGI(TAG, "%s FreeStack: %lu bytes", pcTaskGetName(NULL), \
                         (uint32_t)uxTaskGetStackHighWaterMark(NULL));        \
        }                                                                     \
    } while (0)

#ifdef __cplusplus
extern "C" {
#endif

int app_helper_test(void);
void app_helper_AddrToIpStr(char* buf, size_t len, const app_server_Address_t* addr);
void app_helper_MgAddrToAuthorityStr(char* buf, size_t len, struct mg_addr* addr);
void app_helper_AuthorityStrToAddr(char* buf, size_t len, app_server_Address_t* addr);
void app_helper_AddrToAuthorityStr(char* buf, size_t len, const app_server_Address_t* addr);

#ifdef __cplusplus
}
#endif

#endif /* @end  _APP_HELPER_H_*/