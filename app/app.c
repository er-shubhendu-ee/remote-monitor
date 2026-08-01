/**
 * @file      app.c
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
#include "app.h"
#include "app_client.h"
#include "app_config.h"
#include "app_gpio.h"
#include "app_helper.h"
#include "app_measurement.h"
#include "app_server.h"
#include "app_sntp.h"
#include "app_storage.h"
#include "app_wifi.h"

//
#include "esp_err.h"
#include "esp_event.h"
#include "nvs_flash.h"

//
#include "FreeRTOSConfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

//
#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#define LOG_LEVEL LOG_LEVEL_DEBUG
#define TAG "APP"

#ifndef BOOL_TO_STR
#define BOOL_TO_STR(flag) ((flag) ? "True" : "False")
#endif

typedef enum {
    APP_EVENT_TYPE_IN_INVALID = 0,
    APP_EVENT_TYPE_OUT_INVALID = 0,

    APP_EVENT_TYPE_IN_STA_GOT_IP,
    APP_EVENT_TYPE_IN_SERVER_STARTED,

    APP_EVENT_TYPE_IN_MAX = 0xFF,
    APP_EVENT_TYPE_OUT_MAX = 0xFF
} app_EventType_t;

typedef struct {
    app_EventType_t eventType;
    union {
        struct {
            uint8_t* pData;
            uint32_t dataSize;
        } messageData;
        app_server_Address_t listenData;
    } param;
} app_EventMessage_t;

typedef struct {
    union {
        uint8_t* pBuff;
        uint32_t buffSize;
    } dataBuff;
} app_StateInstanceParam_t;

//
static QueueHandle_t ghEventQueue;

// storage
static char app_gStorageRootPath[] = APP_CONFIG_STORAGE_ROOT_PATH;

//
static void app_WifiEventCb(app_wifi_EventMessage_t* pEventMessage, int isFromIsr);
static void app_ServerEventCb(app_server_EventMessage_t* pEventMessage, int isFromIsr);

//
static int app_PostEvent(app_EventMessage_t* pEventMessage, bool isFromIsr);
static int app_TestWebApp(const char* pBasePathStrSz);
static int app_InitWifi(void);
static int app_InitServer(void);

//
// static void app_IdleState(app_StateInstanceParam_t *pStateInstance, app_EventMessage_t
// *pEventMessage);  TODO: implement state machine

//
static void app_EventStateManagerTask(void* pvParams);

int app_Init(void) {
    int exeStatus = NO_ERROR;

    SERVICE_LOGI(TAG, "FW ver: %s", FW_VERSION);

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ghEventQueue = xQueueCreate(APP_CONFIG_EVENT_QUEUE_ELEMENT_COUNT, sizeof(app_EventMessage_t));
    if (!ghEventQueue) {
#if LOG_LEVEL > LOG_LEVEL_NONE
        SERVICE_LOGE(TAG, "Low memory, can not run the App");
#endif
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    if (pdPASS == xTaskCreate(app_EventStateManagerTask, "Event dispatcher and state manager task",
                              APP_CONFIG_APP_EVENT_MANAGER_TASK_STACK_SIZE, NULL, tskIDLE_PRIORITY,
                              NULL)) {
        exeStatus = NO_ERROR;
    } else {
        exeStatus = ERROR_NOT_ENOUGH_MEMORY;
    }

    APP_HELPER_ASSERT(exeStatus, "Event dispatcher init OK.", "Event dispatcher init fail.");

    APP_HELPER_ASSERT((exeStatus = app_storage_Init(app_gStorageRootPath)), "Storage init OK.",
                      "Storage init fail.");

    APP_HELPER_ASSERT((exeStatus = app_TestWebApp(app_gStorageRootPath)), "Webapp ok.",
                      "Webapp unavailable.");

    APP_HELPER_ASSERT((exeStatus = app_InitWifi()), "Wifi init OK.", "Wifi init fail.");

    APP_HELPER_ASSERT((exeStatus = app_InitServer()), "Server init OK.", "Server init fail.");

    return exeStatus;
}

static void app_WifiEventCb(app_wifi_EventMessage_t* pEventMessage, int isFromIsr) {
    if (!pEventMessage) {
        SERVICE_LOGE(TAG, "NULL pointer to event param");
        return;
    }

    switch (pEventMessage->eventType) {
        case APP_WIFI_EVENT_TYPE_OUT_STA_GOT_IP: {
#if LOG_LEVEL >= LOG_LEVEL_INFO
            SERVICE_LOGI(TAG, "STA got an IP: %s", pEventMessage->param.dataBuff.pBuff);
#endif
            app_EventMessage_t eventParam;
            eventParam.eventType = APP_EVENT_TYPE_IN_STA_GOT_IP;
            app_helper_AuthorityStrToAddr((char*)pEventMessage->param.dataBuff.pBuff,
                                          pEventMessage->param.dataBuff.buffSize,
                                          &eventParam.param.listenData);

            // Bind to all available IP
            memset(eventParam.param.listenData.ip, 0,
                   sizeof(eventParam.param.listenData.ip));  // 0.0.0.0

            eventParam.param.listenData.port = 80;
            eventParam.param.listenData.isIpV6 = 0;
            eventParam.param.listenData.ipV6Scope = 0;

            app_PostEvent(&eventParam, isFromIsr);
            break;
        }

        default: {
            break;
        }
    }
}

static void app_ServerEventCb(app_server_EventMessage_t* pEventMessage, int isFromIsr) {
    if (!pEventMessage) {
#if LOG_LEVEL > LOG_LEVEL_NONE
        SERVICE_LOGE(TAG, "NULL pointer to event param");
#endif
        return;
    }

    if ((APP_SERVER_EVENT_TYPE_UNDEFINED >= pEventMessage->eventType) ||
        (APP_SERVER_EVENT_TYPE_MAX <= pEventMessage->eventType)) {
        /* code */
    }

    switch (pEventMessage->eventType) {
        case APP_SERVER_EVENT_TYPE_OUT_SERVER_STARTED: {
            break;
        }

        case APP_SERVER_EVENT_TYPE_OUT_LISTENING_STARTED: {
#if LOG_LEVEL >= LOG_LEVEL_DEBUG
            SERVICE_LOGD(TAG, "Started listening on: ");  // TODO: add ip to log msg
#endif
            break;
        }

        case APP_SERVER_EVENT_TYPE_OUT_LISTENING_START_ERROR: {
            break;
        }

        case APP_SERVER_EVENT_TYPE_OUT_LISTENING_STOPPED: {
            break;
        }

        case APP_SERVER_EVENT_TYPE_OUT_LISTENING_STOP_ERROR: {
            break;
        }

        case APP_SERVER_EVENT_TYPE_OUT_CONTROL_COMMAND: {
            break;
        }

        default: {
            break;
        }
    }
}

static int app_PostEvent(app_EventMessage_t* pEventMessage, bool isFromIsr) {
    BaseType_t xResult;

    if (isFromIsr) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xResult = xQueueSendFromISR(ghEventQueue, pEventMessage, &xHigherPriorityTaskWoken);

        if (xHigherPriorityTaskWoken == pdTRUE) {
            portYIELD_FROM_ISR();
        }

        return (xResult == pdPASS) ? NO_ERROR : ERROR_INVALID_ACCESS;
    } else {
        xResult = xQueueSend(ghEventQueue, pEventMessage, portMAX_DELAY);
        return (xResult == pdPASS) ? NO_ERROR : ERROR_INVALID_ACCESS;
    }
}

static int app_TestWebApp(const char* pBasePathStrSz) {
    char index_path[128];
    snprintf(index_path, sizeof(index_path), "%s/index.html", app_gStorageRootPath);

    FILE* f = fopen(index_path, "r");
    if (!f) {
        APP_HELPER_ASSERT(ERROR_FILE_NOT_FOUND, "index.html present at %s",
                          "index.html NOT found at %s", index_path);
        return ERROR_FILE_NOT_FOUND;
    }

    // Success path
    APP_HELPER_ASSERT(NO_ERROR, "index.html is accessible at %s", "Unexpected error at %s",
                      index_path);

    char buf[128];
    size_t read_bytes = fread(buf, 1, sizeof(buf) - 1, f);
    buf[read_bytes] = '\0';
    SERVICE_LOGI(TAG, "Preview of index.html:\n%s", buf);
    fclose(f);

    return NO_ERROR;
}

static int app_InitWifi(void) {
    int ret;

    ret = app_wifi_Init(app_WifiEventCb);
    APP_HELPER_ASSERT(ret, "Wi-Fi module init OK", "Wi-Fi module init failed");

    app_wifi_config_t apInitConfig = {.ap.ssidTarget = {APP_CONFIG_AP_SSID_DEFAULT},
                                      .ap.passTarget = {APP_CONFIG_AP_PASS_DEFAULT},
                                      .ap.channel = APP_CONFIG_AP_CHNL_DEFAULT,
                                      .ap.connectionMax = APP_CONFIG_AP_CONN_MAX,
                                      .sta.ssidTarget = {APP_CONFIG_STA_SSID_DEFAULT},
                                      .sta.passTarget = {APP_CONFIG_STA_PASS_DEFAULT}};
    ret = app_wifi_Start(&apInitConfig);
    APP_HELPER_ASSERT(ret, "AP started OK", "Failed to start AP");

    return NO_ERROR;
}

static int app_InitServer(void) {
    int exeStatus = NO_ERROR;
    app_server_Configuration_t configuration;
    app_server_Init(&configuration, app_ServerEventCb);
    return exeStatus;
}

static void app_EventStateManagerTask(void* pvParams) {
    (void)pvParams;
    while (1) {
        APP_HELPER_LOG_FREE_STACK_AT_INTERVAL(TAG, 60000);  // log every 1000 ms
        app_EventMessage_t eventMsg;
        BaseType_t queueReturned = xQueueReceive(ghEventQueue, &eventMsg, portMAX_DELAY);
        if (pdFALSE == queueReturned) {
            continue;
        }

        switch (eventMsg.eventType) {
            case APP_EVENT_TYPE_IN_STA_GOT_IP: {
                SERVICE_LOGI(TAG, "STA connected.");
                // Synchronize time
                app_sntp_Sync();

                // Get current time
                time_t now;
                time(&now);
#if LOG_LEVEL >= LOG_LEVEL_DEBUG
                SERVICE_LOGI("TIME", "Current time: %s", ctime(&now));
#endif

                app_server_EventMessage_t serverEventMsg;
                serverEventMsg.eventType = APP_SERVER_EVENT_TYPE_IN_START_LISTENING;
                memcpy(&serverEventMsg.param.listenData, &eventMsg.param.listenData,
                       sizeof(app_server_Address_t));
                app_server_PostEvent(&serverEventMsg, false);
                break;
            }

            default: {
                break;
            }
        }
    }
}

/* Idle hook: non-blocking, periodic log. Do NOT call vTaskDelay() here. */
void vApplicationIdleHook(void) {
    APP_HELPER_LOG_FREE_STACK_AT_INTERVAL(TAG, 60000);  // log every 1000 ms
}
