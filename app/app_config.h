/**
 * @file      app_config.h
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

#ifndef _APP_CONFIG_H_
#define _APP_CONFIG_H_

#include <stddef.h>
#include <stdint.h>

//
#define APP_CONFIG_EVENT_QUEUE_ELEMENT_COUNT 32

// Storage
#define APP_CONFIG_STORAGE_ROOT_PATH "/spiflash"

// Server
#define APP_CONFIG_SERVER_DNS_DEFAULT "remote-monitor"

// Wifi
#define APP_CONFIG_AP_SSID_DEFAULT "remote-monitor"
#define APP_CONFIG_AP_PASS_DEFAULT "876543210"
#define APP_CONFIG_AP_CHNL_DEFAULT 1
#define APP_CONFIG_AP_CONN_MAX 2

#define APP_CONFIG_STA_SSID_DEFAULT "station-ssid"
#define APP_CONFIG_STA_PASS_DEFAULT "station-passwords"
#define APP_CONFIG_STA_RETRY_CNT_MAX 10

//
#define APP_CONFIG_APP_EVENT_MANAGER_TASK_STACK_SIZE (4U * 1024U)
#define APP_CONFIG_SERVER_TASK_STACK_SIZE (10U * 1024U)

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif /* @end  _APP_CONFIG_H_*/