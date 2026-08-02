/**
 * @file      app_server.c
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
#include "config_app.h"
#include "app_helper.h"
#include "app_server.h"

//
#include "mdns.h"

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

#define LOG_LEVEL LOG_LEVEL_DEBUG
#define TAG "APP_SERVER"

#define EVENT_QUEUE_SIZE_MAX 32

//
#define MG_POLL_TIMEOUT 1000  // in ms

//
#define STACK_LOG_INTERVAL 60000  // in ms

typedef struct {
    struct mg_mgr manager;
    struct mg_connection* pConnection;
    struct {
        uint8_t isWebSocketConnected : 1;
        uint8_t isNetStatUpdated : 1;
    } status;
} connectionStruct_t;

// Internal
static QueueHandle_t ghEventQueue;
static connectionStruct_t gApConnection;

// External
static app_server_cb_t gEventCb = NULL;

//
static void EventHandler(app_server_EventMessage_t* pEventMessage, int isFromIsr);
static void MongooseEventHandler(struct mg_connection* c, int ev, void* ev_data);
static void SendNetworkState(struct mg_connection* c, bool state);

//
static void ServerTask(void* pvParam);

//

int app_server_Init(app_server_Configuration_t* pConfiguration, app_server_cb_t eventCb) {
    ghEventQueue = xQueueCreate(EVENT_QUEUE_SIZE_MAX, sizeof(app_server_EventMessage_t));

    gEventCb = eventCb;

    if (!ghEventQueue) {
#if LOG_LEVEL > LOG_LEVEL_NONE
        SERVICE_LOGE(TAG, "Failed to create event queue.");
#endif
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    xTaskCreate(ServerTask, "Server task", APP_CONFIG_SERVER_TASK_STACK_SIZE, NULL,
                tskIDLE_PRIORITY + 1, NULL);
    mg_mgr_init(&gApConnection.manager);  // Initialise event manager

    return NO_ERROR;
}

int app_server_DeInit(void) { return NO_ERROR; }

int app_server_PostEvent(app_server_EventMessage_t* pEventMessage, bool isFromIsr) {
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

// ---------------------------------------------------------------------
// Event handler
// ---------------------------------------------------------------------
static void EventHandler(app_server_EventMessage_t* pEventMessage, int isFromIsr) {
    if (!pEventMessage) {
        return;
    }

    if ((APP_SERVER_EVENT_TYPE_UNDEFINED >= pEventMessage->eventType) ||
        (APP_SERVER_EVENT_TYPE_MAX <= pEventMessage->eventType)) {
        return;
    }

    switch (pEventMessage->eventType) {
        case APP_SERVER_EVENT_TYPE_IN_START_LISTENING:
            char strBuffTemp[64];

            mdns_init();
            mdns_hostname_set(APP_CONFIG_SERVER_DNS_DEFAULT);
            mdns_instance_name_set("Gomukh ESP32 Server");

            memset(strBuffTemp, 0, sizeof(strBuffTemp));
            app_helper_AddrToAuthorityStr(strBuffTemp, sizeof(strBuffTemp),
                                          &pEventMessage->param.listenData);

#if LOG_LEVEL >= LOG_LEVEL_DEBUG
            SERVICE_LOGD(TAG, "Trying to start listening on %s", strBuffTemp);
#endif
            struct mg_connection* pMgConnection =
                mg_http_listen(&gApConnection.manager, strBuffTemp, MongooseEventHandler, NULL);
            // mg_http_listen(&gApConnection.manager, "192.168.4.1:80", MongooseEventHandler,
            //                NULL);  // TODO: implement in app supplies AP IP

            app_server_EventMessage_t eventMsg;
            if (pMgConnection) {
#if LOG_LEVEL >= LOG_LEVEL_DEBUG
                SERVICE_LOGD(TAG, "Started listening on: ");  // TODO: add ip to log msg
#endif
                eventMsg.eventType = APP_SERVER_EVENT_TYPE_OUT_LISTENING_STARTED;
            } else {
#if LOG_LEVEL >= LOG_LEVEL_DEBUG
                SERVICE_LOGD(TAG, "Listening start error on: ");  // TODO: add ip to log msg
#endif
                eventMsg.eventType = APP_SERVER_EVENT_TYPE_OUT_LISTENING_START_ERROR;
            }

            if (gEventCb) {
                gEventCb(&eventMsg, xPortInIsrContext());
            }

            memset(strBuffTemp, 0, sizeof(strBuffTemp));
            app_helper_AddrToIpStr(strBuffTemp, sizeof(strBuffTemp),
                                   &pEventMessage->param.listenData);

            break;

        default:
            break;
    }
}

static void MongooseEventHandler(struct mg_connection* c, int ev, void* ev_data) {
    char ipbuf[32];  // for logging
    app_helper_MgAddrToAuthorityStr(ipbuf, sizeof(ipbuf), &c->rem);

    switch (ev) {
        // 1️⃣ New connection opened (optional hex dump)
        case MG_EV_OPEN:
            // c->is_hexdumping = 1;  // Uncomment to log raw traffic
            gApConnection.pConnection = c;
            break;

        // 2️⃣ HTTP request received
        case MG_EV_HTTP_MSG: {
            struct mg_http_message* hm = (struct mg_http_message*)ev_data;
            struct mg_http_serve_opts opts = {.root_dir = "/spiflash", .fs = &mg_fs_posix};

            // WebSocket upgrade request
            if (mg_match(hm->uri, mg_str("/ws"), NULL)) {
                SERVICE_LOGI(TAG, "WebSocket upgrade requested from %s", ipbuf);
                mg_ws_upgrade(c, hm, NULL);
                return;
            }

            // REST endpoint
            if (mg_match(hm->uri, mg_str("/api/hello"), NULL)) {
                mg_http_reply(c, 200, "Content-Type: application/json\r\n",
                              "{\"status\":1,\"msg\":\"hello\"}");
                return;
            }

            // Serve static files
            mg_http_serve_dir(c, hm, &opts);
            break;
        }

        // 3️⃣ WebSocket connection opened
        case MG_EV_WS_OPEN: {
            SERVICE_LOGI(TAG, "✅ WebSocket connected from %s", ipbuf);

            // Send welcome message

            const char* hello = "{\"event\":\"connected\",\"status\":\"ok\"}";
            mg_ws_send(c, hello, strlen(hello), WEBSOCKET_OP_TEXT);
            gApConnection.status.isWebSocketConnected = true;
            break;
        }

        // 4️⃣ WebSocket message received
        case MG_EV_WS_MSG: {
            struct mg_ws_message* wm = (struct mg_ws_message*)ev_data;

            SERVICE_LOGD(TAG, "📩 WS RX: %.*s", (int)wm->data.len, wm->data.buf);

            // Echo back
            // mg_ws_send(c, wm->data.buf, wm->data.len, WEBSOCKET_OP_TEXT);

            break;
        }

        // 5️⃣ Connection closed
        case MG_EV_CLOSE:
            if (c->is_websocket) {
                SERVICE_LOGI(TAG, "❌ WebSocket closed (%s)", ipbuf);
            } else if (c->is_accepted) {
                SERVICE_LOGI(TAG, "🔌 HTTP connection closed (%s)", ipbuf);
            }
            gApConnection.pConnection = NULL;
            gApConnection.status.isWebSocketConnected = false;
            gApConnection.status.isNetStatUpdated = false;
            break;

        default:
            break;
    }
}

static void ServerTask(void* pvParam) {
    while (1) {
        app_server_EventMessage_t eventMessage;
        BaseType_t queueReturned = xQueueReceive(ghEventQueue, &eventMessage, 0);

        if (pdTRUE == queueReturned) {
            EventHandler(&eventMessage, false);
        }

        mg_mgr_poll(&gApConnection.manager, MG_POLL_TIMEOUT);  // Infinite event loop

        APP_HELPER_LOG_FREE_STACK_AT_INTERVAL(TAG, STACK_LOG_INTERVAL);  // log every 1000 ms

        // if (gApConnection.status.isWebSocketConnected && !gApConnection.status.isNetStatUpdated)
        // {
        //     SendNetworkState(gApConnection.pConnection, true);
        //     gApConnection.status.isNetStatUpdated = true;
        // }
    }
}

static void SendNetworkState(struct mg_connection* c, bool state) {
    if (c && c->is_websocket) {
        mg_ws_printf(c, WEBSOCKET_OP_TEXT, "{\"event\":\"network_state\",\"value\":%d}",
                     state ? 1 : 0);
    }
}
