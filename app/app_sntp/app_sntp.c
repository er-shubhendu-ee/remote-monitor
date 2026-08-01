/**
 * @file      app_sntp.c
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

#include "util_common_error.h"
#include "util_common_log.h"

//
#include "app_sntp.h"

//
#include "esp_sntp.h"
#include "esp_system.h"

//
#include "FreeRTOSConfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

//
#include <time.h>

#define LOG_LEVEL LOG_LEVEL_DEBUG
#define TAG "APP_SNTP"

static int app_sntp_Init(void);

int app_sntp_Sync(void) {
    app_sntp_Init();

    // Wait for time to be set
    time_t now = 0;
    struct tm timeinfo = {0};
    int retry = 0;
    const int retry_count = 10;

    while (timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count) {
#if LOG_LEVEL >= LOG_LEVEL_DEBUG
        SERVICE_LOGI("TIME", "Waiting for system time to be set... (%d/%d)", retry, retry_count);
#endif
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        time(&now);
        localtime_r(&now, &timeinfo);
    }

    if (timeinfo.tm_year >= (2016 - 1900)) {
#if LOG_LEVEL >= LOG_LEVEL_INFO
        SERVICE_LOGI("TIME", "Time is set!");
#endif
        return NO_ERROR;
    } else {
#if LOG_LEVEL > LOG_LEVEL_NONE
        SERVICE_LOGE("TIME", "Failed to get time from NTP");
#endif
        return ERROR_INFO_NOT_AVAIL;
    }
}

static int app_sntp_Init(void) {
    SERVICE_LOGI("TIME", "Initializing SNTP");

    // Set operating mode
    sntp_setoperatingmode(SNTP_OPMODE_POLL);

    // Set NTP server(s)
    sntp_setservername(0, "pool.ntp.org");   // Primary NTP server
    sntp_setservername(1, "time.nist.gov");  // Optional secondary server

    // Initialize
    sntp_init();

    return NO_ERROR;
}